#include <stdint-gcc.h>
#include <stddef.h>
#include <stdio.h>

#include "malloc.h"
#include "block_device.h"
#include "mbr.h"

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
    uint32_t largeSectCnt;
};

struct FAT_EBR
{
    uint32_t scetorsPerFat;
    uint16_t flags;
    uint16_t versionNum;
    uint32_t clusterRootNum;
    uint16_t sectNumFSInfo;
    uint16_t secNumBKBS;
    uint8_t sig;
    char label[11];
    uint16_t bootSig;
};

struct  FAT_DIRENT
{
    char FileName[11];
    uint8_t AttrFlag;
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
};
 
struct FAT_LONG_FILE_NAME
{
    char FileName[11];
    uint8_t AttrFlag;
    uint8_t lgType;
    uint8_t checkSum;
    char FileName2[12];
    uint16_t reserved;
    char FileName3[4];
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
 
    //Read in the mnumber of sectors per FAT
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
 
    //Read in large sector count
    bpb->largeSectCnt = readPtr[3] << 24 | readPtr[2] << 16 |readPtr[1] << 8 |readPtr[0];
	readPtr+=4;
	
	return bpb;
}

struct FAT_EBR * fat_loadebr(void * buffer)
{
	struct FAT_EBR * ebr = (struct FAT_EBR*)malloc(sizeof(struct FAT_EBR));
	 
	 uint8_t * readPtr = (uint8_t*)buffer;
	 readPtr +=36; //EBR offset after Bios param  block
 
    //Read in sectors per file allocation table
    ebr->scetorsPerFat = readPtr[3] << 24 | readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
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

void init_fatfs(struct BLOCK_DEVICE * device)
{
	uint8_t * readbuffer = (uint8_t*)malloc(sizeof(uint8_t) * 512);
	uint32_t bytesRead = device->read(&readbuffer, 0);
	
	struct MASTER_BOOT_RECORD * mbr = LoadBootRecord(&readbuffer);
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
	memset(fatBuffer, 0 , 512);
	
	device->read(&fatBuffer, tlbentry->LBA); 

	struct FAT_BPB * bpb = fat_loadbpb(&fatBuffer);
	struct FAT_EBR * ebr = fat_loadebr(&fatBuffer);
	printf("Sig value: 0x%h \n", ebr->bootSig);
}

