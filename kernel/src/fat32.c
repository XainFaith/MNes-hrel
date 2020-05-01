#include <stdint-gcc.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "malloc.h"
#include "block_device.h"
#include "mbr.h"
#include "vfs.h"

#include "fat32.h"

struct FAT_BPB
{
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors; //Boot record sectors are included in the reserved sectors
    uint8_t fatCnt;
    uint16_t numDirent;   //Number of directory entries (must be set so that the root directory occupies entire sectors).
    uint16_t totSectors; //The total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume,
                         // and the actual count is stored in the Large Sector Count entry at 0x20
    uint16_t sectorsPerFat;
    uint16_t numSectorsPerTrack;
    uint32_t numHiddenSectors;
	uint32_t largeSectorCount;
};

struct FAT_EBR
{
    uint32_t sectorsPerFat;
    uint16_t flags;
    uint16_t versionNum;
    uint32_t clusterRootNum;
    uint16_t sectNumFSInfo;
    uint16_t secNumBKBS;
    uint8_t sig;
    char label[11];
    uint16_t bootSig;
};
 
struct FAT_ENTRY_LIST_NODE
{
    struct FAT_DIRENT * dirent;
    struct FAT_ENTRY_LIST_NODE * next;
};

struct FAT_BPB * fat_loadbpb(void * buffer)
{
	struct FAT_BPB * bpb = malloc(sizeof(struct FAT_BPB));
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

struct FAT_EBR * fat_loadebr(void * buffer)
{
	struct FAT_EBR * ebr = (struct FAT_EBR*)malloc(sizeof(struct FAT_EBR));
	 
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

typedef struct
{
	unsigned char * fattlb;
	struct BLOCK_DEVICE * dev;
	struct FAT_BPB * bpb;
	struct FAT_EBR * ebr;
	uint32_t firstdatalba;
}fat_fs;

typedef struct 
{
    char name[11];
    uint8_t attrflag;
    uint8_t reserved;
    uint8_t ctTenths;
    uint16_t ct;
    uint16_t dt;
    uint16_t lastAccessed;
    uint16_t addrHighbits;
    uint16_t lastModTime;
    uint16_t lastModDate;
    uint16_t addrLowBits;
    uint32_t sizeInBytes;
}fat_dirent;

typedef struct
{
	fat_dirent * dirent;
	char * name;
} fat_dirent_node;

typedef struct
{
    uint8_t sequence;
    char name[10];
    uint8_t attrflag;
	uint8_t type;
	uint8_t checksum;
    char name2[12];
    uint16_t reserved;
    char name3[4];
} fat_lent;

int fat_read_fattlb(uint32_t sectorstofat,  uint32_t lba, unsigned char * tlbbuffer,struct BLOCK_DEVICE * device)
{
	uint32_t bytesread = 0;
	for(int i =0;  i < sectorstofat; i++)
	{
		device->read(tlbbuffer,lba);
		lba++;
		tlbbuffer += 512;
		bytesread+= 512;
	}
	return bytesread;
}

uint32_t fat_clustertolba(fat_fs * fs, uint32_t clusternum)
{
	return fs->firstdatalba + (fs->bpb->sectorsPerCluster * (clusternum -2));
}

char * fat_getdirentname(fat_dirent * dirent)
{
	char * name = (char*)malloc(11);
	
	uint8_t lastindex = 0;
	for(int i =0; i < 11; i++)
	{
		if(dirent->name[i] != 0)
		{
			name[lastindex] = dirent->name[i];
			lastindex++;
		}
	}
	
	return name;
}

char * fat_getlongname(fat_lent * lent, uint32_t entcount)
{
	char * name = (char*)malloc(entcount * 26);
	bool process = true;
	uint32_t charcount = 0;
	while(process)
	{
		for(int i = 0; i < 10; i++)
		{
			if(lent->name[i] != 0xFF && lent->name[i] != 0x0)
			{
				name[charcount] = lent->name[i];
				charcount++;
			}
		}
		
		for(int i =0; i <12; i ++)
		{
			if(lent->name2[i] != 0xFF && lent->name2[i] != 0x0)
			{
				name[charcount] = lent->name2[i];
				charcount++;
			}
		}
		
		for(int i =0; i <4; i ++)
		{
			if(lent->name3[i] != 0xFF && lent->name3[i] != 0x0)
			{
				name[charcount] = lent->name3[i];
				charcount++;
			}
		}
		
		if(lent->sequence & 0x40)
		{
			process = false;
			charcount++;
			name[charcount] = '/0';
		}
		else
		{
			lent -= sizeof(fat_lent);
		}
	}
	
	return name;
}

uint32_t fat_getdircluster(char * dirname, fat_fs * fs, uint32_t clusternum)
{
	unsigned char * clusterdata = (unsigned char *)malloc(fs->bpb->sectorsPerCluster * fs->bpb->bytesPerSector);
	uint32_t clusterlba = fat_clustertolba(fs, clusternum);

	fs->dev->readblocks(clusterdata,clusterlba, fs->bpb->sectorsPerCluster);
	
	fat_dirent * dirent = (fat_dirent*)(uint32_t)clusterdata;
	fat_lent * longent = NULL;
	uint32_t longentcount = 0;
	uint32_t readindex = 0;
	
	while(dirent != NULL)
	{
		dirent = (fat_dirent*)(((uint32_t)clusterdata) + readindex);
		//If the first byte of the cluster is 0x0 then the cluster is empty
		if(dirent->name[0] == 0x0)
		{
			return 0x0; //TODO set an error flag or something
		}
	
		if(dirent->attrflag != 0xe5)
		{
			    //Does this entry contain a director entry
				if(dirent->attrflag == 0x10)
				{
					
					char * direntname = NULL;
					
					//If longent is not null there are long file name entries to the directory
					if(longent != NULL)
					{
						direntname = fat_getlongname(longent, longentcount);
					}
					else
					{
						direntname = fat_getdirentname(dirent);
					}

					//Is this the directory we are looking for
					if(strcmp(direntname,dirname) == 0)
					{
							uint32_t dircluster = (dirent->addrHighbits << 16)  | (dirent->addrLowBits);
							free(clusterdata);
							free(direntname);
							return dircluster;
					}

					free(direntname);
				}
				else if(dirent->attrflag == 0x0f) //Is the entry a long file name entry
				{
						longent = (fat_lent*)dirent;
						if(longent->sequence & 0x40)
						{
							longentcount = longent->sequence - 0x40;
						}
						else
						{
							longent = NULL;
						}
				}
		}
		
		readindex+=sizeof(fat_dirent);
		
		if(readindex >= fs->bpb->bytesPerSector * fs->bpb->sectorsPerCluster)
		{
				return 0x0; //dir not found so return culster number 0 which is not a valid cluster
		}
	}
}

vfs_dirent fat_opendir(char * path, fat_fs * fs)
{
	
	if(path == NULL)
	{
		return NULL;
	}
	
	//Get Root Direction
	unsigned char * clusterdata = (unsigned char *)malloc(fs->bpb->sectorsPerCluster * fs->bpb->bytesPerSector);
	uint32_t clusterlba = fat_clustertolba(fs,fs->ebr->clusterRootNum);
	fs->dev->readblocks(clusterdata,clusterlba, fs->bpb->sectorsPerCluster);

	//Are we trying to open the root directory if we are then we can just return the root cluster number
	if(strcmp(path, "/") == 0x0)
	{
		return fs->ebr->clusterRootNum;
	}
	
	char * dir = splitpath(path, '/');
	
	uint32_t clusternum = fs->ebr->clusterRootNum;
	while(dir != NULL)
	{
		clusternum = fat_getdircluster(dir, fs, clusternum);
		
		//If we are still in the loop and clusternum is 0x0 then the dir was not found
		if(clusternum == 0x0)
		{
			return NULL;
		}
		
		free(dir);
		dir = splitpath(NULL, '/');
	}
	
	return clusternum;
	
}

fs_driver * init_fatfs(struct BLOCK_DEVICE * device)
{
	uint8_t * readbuffer = (uint8_t*)malloc(sizeof(uint8_t) * 512);
	uint32_t bytesread = device->read(readbuffer, 0);
	
	struct MASTER_BOOT_RECORD * mbr = LoadBootRecord(readbuffer);
	struct PARTITION_TLB_ENTRY * tlbentry = NULL;
	
	//Only expecting one Fat32 Partition but it may not be the first one listed in the tlb entries
	for(int i =0; i < 4; i++)
	{
		//Check for Fat32 file systems
		if(mbr->tlbEntries[i]->partitionType == 0xB | mbr->tlbEntries[i]->partitionType == 0xC)
		{
				tlbentry = mbr->tlbEntries[i];
				break;
		}
	}
	
	uint8_t * fatBuffer = (uint8_t*)malloc(512);
	device->read(fatBuffer, tlbentry->LBA); 

	struct FAT_BPB * bpb = fat_loadbpb(fatBuffer);
	struct FAT_EBR * ebr = fat_loadebr(fatBuffer);
		
	fat_fs * fatfs = (fat_fs*)malloc(sizeof(fat_fs));
	fatfs->fattlb = (unsigned char*)malloc(bpb->bytesPerSector * ebr->sectorsPerFat);
	//Calculate the LBA of the FAT
	uint32_t fatlba = tlbentry->LBA + bpb->reservedSectors;
	bytesread = fat_read_fattlb(ebr->sectorsPerFat, fatlba, fatfs->fattlb, device);
	
	fatfs->bpb = bpb;	
	fatfs->ebr = ebr;
	fatfs->firstdatalba = fatlba + (ebr->sectorsPerFat * bpb->fatCnt);
	fatfs->dev = device;
	
	fs_driver * fat_driver = (fs_driver*)malloc(sizeof(fs_driver));
	
	fat_driver->native_driver = fatfs;
	fat_driver->fsopendir = &fat_opendir;
		
	return fat_driver;
}


