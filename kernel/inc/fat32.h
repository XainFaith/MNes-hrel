#ifndef _FAT_32_H
#define _FAT_32_H

#include <stdint-gcc.h>
#include "malloc.h"
#include "block_device.h"
#include "mbr.h"
#include "vfs.h"
#include "dirent.h"

#define FAT_ARCHIVE_ENT 0x20
#define FAT_DIR_ENT 0x10
#define FAT_LONG_NAME_ENT 0x0F

typedef struct  fat_bpb
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
} fat_bpb;

typedef struct fat_ebr
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
}fat_ebr;
 
typedef struct fat_fs
{
	uint32_t * fattlb;
	block_device * dev;
	fat_bpb* bpb;
	fat_ebr * ebr;
	uint32_t firstdatalba;
	
	uint32_t clustersize;
}fat_fs;

typedef struct fat_lent
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

typedef struct fat_lent_node
{
	fat_lent * lent;
	struct fat_lent_node * prev;
} fat_lent_node;

typedef struct  fat_dirent
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

typedef struct fat_dir{
	char * data;
	char * readptr;
	uint32_t size;
} fat_dir;

fs_driver * init_fatfs(block_device * device);
uint32_t fat_clustertolba(fat_fs * fs, uint32_t clusternum);
int fat_getclustercount(uint32_t clusterroot, fat_fs * fs);
char * fat_getdata(uint32_t clusterroot, uint32_t * sizeout, fat_fs * fs);
int fat_getdirent(char * buffer,  dirent * out);


DIR * fat_opendir(const char * path, fat_fs * driver);
void fat_rewinddir(fat_dir * dir);
dirent * fat_readdir(fat_dir * dir);
int fat_closedir(fat_dir * dir, fat_fs * fs);

vfs_open_file * fat_fopen(char * path, fat_fs * fs);
size_t fat_fread(unsigned char *ptr, size_t size, vfs_open_file * file, fat_fs * fs);




#endif