#include <stdint-gcc.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "fat32.h"
#include "dirent.h"

fat_bpb* fat_loadbpb(void * buffer)
{
	fat_bpb * bpb = (fat_bpb*)malloc(sizeof(fat_bpb));
	uint8_t * readPtr = (uint8_t*)buffer;
    //Jump to relative data
    readPtr+=11;
 
    //Read in the number of bytes per sector
    bpb->bytesPerSector = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Read in the number of sectors per cluster
    bpb->sectorsPerCluster = readPtr[0];
    readPtr++;
 
    //Read in the number of reservered sectors
    bpb->reservedSectors = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Read in the number of File allocation tables
    bpb->fatCnt = readPtr[0];
    readPtr++;
 
    //Read in the number of directory entires
    bpb->numDirent = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Read in the total number of sectors in the volume if 0 it means there is more then 65535 sectors in the volume and the large sector count entry is at 0x20
    bpb->totSectors = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Skip media descriptor byte
    readPtr++;
 
    //sectors per FAT as its used for fat12/16
	bpb->sectorsPerFat = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Read in number of sectors per track
    bpb->numSectorsPerTrack = readPtr[1] << 8 | readPtr[0];
    readPtr++;
 
    //Skip over head count
    readPtr+=2;
 
    //Read in number of hidden sectors
    bpb->numHiddenSectors = readPtr[3] << 24 | readPtr[2] << 16 |readPtr[1] << 8 |readPtr[0];
    readPtr+=4;
 
    //Read in large sector count for the total count of sectors in volume if it exceeds the totSectors capcity to hold
	bpb->largeSectorCount = readPtr[3] << 24 | readPtr[2] << 16 |readPtr[1] << 8 |readPtr[0];
	
	readPtr+=4;
	
	return bpb;
}

fat_ebr * fat_loadebr(void * buffer)
{
	fat_ebr * ebr = (fat_ebr*)malloc(sizeof(fat_ebr));
	 
	 uint8_t * readPtr = (uint8_t*)buffer;
	 readPtr +=36; //EBR offset after Bios param  block
 
    //Read in sectors per file allocation table
    ebr->sectorsPerFat = readPtr[3] << 24 | readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
    readPtr += 4;
 
    //Read in flags
    ebr->flags = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
   
    //Read in Fat Version number
    ebr->versionNum = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Read in the cluster number of the root directory
    ebr->clusterRootNum = readPtr[3] << 24 | readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
    readPtr += 4;
 
    //Read in the sector number of the FSinfo structure
    ebr->sectNumFSInfo = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Read in the sector number of the backup boot sector
    ebr->secNumBKBS = readPtr[1] << 8 | readPtr[0];
    readPtr+=2;
 
    //Jump over some less usfull info
    readPtr += 14;
 
    //Read in the signature value should be 0x28 or 0x29
    ebr->sig = readPtr[0];
    readPtr++;
 
    //Jump over the drive number
    readPtr+=4;
 
    //Copy the label of the partition
    for(int i = 0; i < 11; i++)
    {
        ebr->label[i] = readPtr[i];
    }
    readPtr += 11;
 
    //Jump over some more less usfull data
    readPtr += 428;
   
    //Read in boot parition sig
    ebr->bootSig = readPtr[1] << 8 | readPtr[0];
	
	return ebr;
}

uint32_t fat_clustertolba(fat_fs * fs, uint32_t clusternum)
{
	return fs->firstdatalba + (fs->bpb->sectorsPerCluster * (clusternum -2));
}

fs_driver * init_fatfs(block_device * device)
{
	uint8_t * readbuffer = (uint8_t*)malloc(sizeof(uint8_t) * 512);
	uint32_t bytesread = device->read(readbuffer, 0);
	
	if(bytesread != 512)
	{
		free(readbuffer);
		return NULL;
	}
	
	master_boot_record  * mbr = LoadBootRecord(readbuffer);
	mbr_partion_tlb_entry * tlbentry = NULL;
	
	//Only expecting one Fat32 Partition but it may not be the first one listed in the tlb entries
	for(int i =0; i < 4; i++)
	{
		//Check for Fat32 file systems
		if(mbr->tlbEntries[i] ->partitionType == 0xB || mbr->tlbEntries[i]->partitionType == 0xC)
		{
				tlbentry = mbr->tlbEntries[i];
				break;
		}
	}
	
	uint8_t * fatBuffer = (uint8_t*)malloc(512);
	device->read(fatBuffer, tlbentry->LBA); 

	fat_bpb* bpb = fat_loadbpb(fatBuffer);
	fat_ebr * ebr = fat_loadebr(fatBuffer);
		
	fat_fs * fatfs = (fat_fs*)malloc(sizeof(fat_fs));
	fatfs->fattlb = (uint32_t*)malloc(bpb->bytesPerSector * ebr->sectorsPerFat);
	//Calculate the LBA of the FAT
	uint32_t fatlba = tlbentry->LBA + bpb->reservedSectors;
	bytesread = device->readblocks((unsigned char*)fatfs->fattlb, fatlba,ebr->sectorsPerFat);
	
	fatfs->bpb = bpb;	
	fatfs->ebr = ebr;
	fatfs->firstdatalba = fatlba + (ebr->sectorsPerFat * bpb->fatCnt);
	fatfs->dev = device;
	fatfs->clustersize = bpb->bytesPerSector * bpb->sectorsPerCluster;
	
	fs_driver * fat_driver = (fs_driver*)malloc(sizeof(fs_driver));
	fat_driver->native_fs = fatfs;
	fat_driver->fsopendir = &fat_opendir;
	fat_driver->fsreaddir = &fat_readdir;
	fat_driver->fsopenfile = &fat_fopen;
	fat_driver->fsreadfile = &fat_fread;
	fat_driver->fsclosedir = &fat_closedir;
	fat_driver->fsrewinddir = &fat_rewinddir;
		
	return fat_driver;
}

dirent * fat_readdir(fat_dir * dir)
{
	static dirent rdir;
	
	uint32_t readaddr = (uint32_t)dir->readptr;
	uint32_t endofdata = ((uint32_t)dir->data) + dir->size;
	
	if(readaddr < endofdata)
	{
		int next =  fat_getdirent(dir->readptr, &rdir);
		if(next == -1)
		{
			dir->readptr += sizeof(fat_dirent);
			return NULL;
		}
		else
		{
			dir->readptr += next;
		}
		
		return &rdir;
	}	
	
	return NULL;
}

void fat_rewinddir(fat_dir * dir)
{
	dir->readptr = dir->data;
}

/*
 * 	Opens a directory for reading
 */ 
DIR * fat_opendir(const char * path, fat_fs * fs)
{
	if(path == NULL)
	{
		return NULL;
	}

	//Edge case for reading root directory
	if(strcmp(path, "/") == 0x0)
	{
		uint32_t buffersize = 0;
		char * buffer = fat_getdata(fs->ebr->clusterRootNum, &buffersize,fs);
		fat_dir * odir = (fat_dir*)malloc(sizeof(fat_dir));
		odir->data = buffer;
		odir->readptr = buffer;
		odir->size = buffersize;
		return odir;
	}
	
	char * subdir = strtok(path, "/");
	
	uint32_t clusternum = fs->ebr->clusterRootNum;

	uint32_t buffersize = 0;
	char * buffer = fat_getdata(clusternum, &buffersize,fs);
	char * readptr = buffer;
	dirent * dir = (dirent*)malloc(sizeof(dirent));
	
	nextread: while(readptr < (buffer + buffersize))
	{
		
		int nextoffset = fat_getdirent(readptr, dir);
			
		if(dir != NULL)
		{
			if(strcmp(dir->d_name, subdir) == 0x0) //We found the directory we were looking for
			{
				subdir  = strtok(NULL, "/");
				
				if(subdir == NULL) //End of given path so this must be the directory requested to be opened
				{
					readptr += nextoffset - sizeof(fat_dirent);
					fat_dirent * fdir = (fat_dirent*)readptr;
					clusternum = (fdir->addrHighbits << 16) | (fdir->addrLowBits);
					free(buffer);
					free(dir);
					
					buffersize = 0;
					buffer = fat_getdata(clusternum, &buffersize,fs);
					fat_dir * odir = (fat_dir*)malloc(sizeof(fat_dir));
					odir->data = buffer;
					odir->readptr = buffer;
					odir->size = buffersize;
					return odir;
				}
				
				//look in the next directory
				fat_dirent * fdir = (fat_dirent*)readptr;
				clusternum =  ((fdir->addrHighbits) >> 16) |  fdir->addrLowBits;
				
				free(buffer);
				buffersize = 0;
				buffer = fat_getdata(clusternum, &buffersize,fs);
				readptr = buffer;
				goto nextread;
			}
		}
		//Check next offset value and increment read pointer
		if(nextoffset == -1)
		{
			readptr += sizeof(fat_dirent);
		}
		else
		{
			readptr += nextoffset;
		}
	}

	//Directory not found
	if(buffer != NULL)
	{
		free(buffer);
	}
	
	free(dir);
	return NULL;
}

int fat_closedir(fat_dir * dir, fat_fs * fs)
{
	if(dir->data != NULL)
	{
		free(dir->data);
		dir->data = NULL;
	}
	dir->readptr = NULL;
	dir->size = 0;
	return 0;
}

vfs_open_file * fat_fopen(char * path, fat_fs * fs)
{
	return NULL;	
}

size_t fat_fread(unsigned char *ptr, size_t size, vfs_open_file * file, fat_fs * fs)
{
	return -1;	
}

/*
 * 	returns the number of clusters in a chain given the first cluster number in the chain
 */ 
int fat_getclustercount(uint32_t clusterroot, fat_fs * fs)
{
	int count = 1;
	uint32_t clusternum = clusterroot;
	while(1)
	{
		
		if((fs->fattlb[clusternum]   & 0x0FFFFFFF)  >= 0x0FFFFFF8) //End of chain
		{
			return count;
		}
		else
		{
			count++;
			clusternum = fs->fattlb[clusternum] & 0x0FFFFFFF;
		}
	}
	
	return count;
}

/*
 *  returns a buffer to the data for the given cluster chain
 * starting from the cluster root 
 */ 
char * fat_getdata(uint32_t clusterroot, uint32_t * sizeout, fat_fs * fs)
{
	uint32_t clustercount = fat_getclustercount(clusterroot, fs);
	uint32_t clusternum = clusterroot;
	uint32_t readcount = 0;
	//Allocate the space required for the data to be read
	char * data = (char*)malloc(fs->clustersize * clustercount);
	char * bufptr = data;
	
	while(1)
	{
		uint32_t lba  = fat_clustertolba(fs, clusternum);
		readcount += fs->dev->readblocks(bufptr,lba, fs->bpb->sectorsPerCluster);
		clustercount--;
		if(clustercount != 0)
		{
			clusternum = fs->fattlb[clusternum] & 0x0FFFFFFF;
			bufptr += fs->clustersize;
		}
		else
		{
			break;
		}
	}
	*sizeout = readcount;
	return data;
}

/*
*	used to get the next dirent entry from the buffer
* returns the offset to the next entry in the buffer 
* if -1 is returned no entry was found at the current buffer head
* and it should be incremented by size of fat_dirent
*/
int fat_getdirent(char * buffer,  dirent * out)
{
	fat_dirent * fdir = (fat_dirent*)buffer;
	int nextoffset = sizeof(fat_dirent);
	char * nameptr = out->d_name;
	int lentcount = 0;
	//If this is a long name file entry advance past it for now
	if(fdir->attrflag == FAT_LONG_NAME_ENT)
	{
		lentcount++;
		nextoffset += sizeof(fat_lent);
		fdir++;
	}
	
	//Is this a Directory Entry or an Archive Entry
	if(fdir->attrflag == FAT_DIR_ENT || fdir->attrflag == FAT_ARCHIVE_ENT)
	{
		//if there was not long file name entries present for this entry just copy the bits for the name
		if(nextoffset == sizeof(fat_dirent))
		{
			memcpy(out->d_name, fdir->name,11);
			out->d_name[12] = '\0';
		}
		else
		{
			//There are long name file entrys to accompany this entry
			fat_lent  * lent = (fat_lent*)fdir;
			lent--;
			for(int i = lentcount; i > 0; i--)
			{
				for(int i =0; i < lentcount; i++)
				{
					for(int j = 0; j < 10; j++)
					{
						if(lent->name[j] != 0x0 && lent->name[j] != 0xFF)
						{
							*nameptr = lent->name[j];
							nameptr++;
						}
					}
					
					for(int y = 0; y < 12; y++)
					{
						if(lent->name2[y] != 0x0 && lent->name2[y] != 0xFF)
						{
							*nameptr = lent->name2[y];
							nameptr++;
						}
					}
					
					for(int x = 0; x < 4; x++)
					{
						if(lent->name3[x] != 0x0 && lent->name3[x] != 0xFF)
						{
							*nameptr = lent->name3[x];
							nameptr++;
						}
					}

					lent--;
				}
			}

			*nameptr = '\0';
		}
		
		if(fdir->attrflag == FAT_DIR_ENT)
		{
			out->d_type = DT_DIR;
		}
		else
		{
			out->d_type = DT_REG;
		}
		
		return nextoffset;
	}
	out = NULL;
	return -1;
}