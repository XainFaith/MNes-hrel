#ifndef _MBR_H
#define _MBR_H

#include <stdint-gcc.h>

struct MBR_HEADER
{
    uint8_t ins[3];
    uint8_t oem_ident[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t resvered_sectors;
    uint8_t fat_cnt;
    uint16_t num_dir_entries;
    uint16_t tot_sectors;
    uint8_t media_desc_type;
    uint16_t fat_per_sec_f12_f16;
    uint16_t sectors_per_track;
    uint16_t head_cnt;
    uint32_t hidden_sector_cnt;
    uint32_t lrg_sector_cnt;
};

struct PARTITION_TLB_ENTRY
{
    uint32_t paritionAddr;
    uint32_t lastPartAddr;
    uint32_t LBA;
    uint32_t sectorCnt;
    uint8_t partitionType;
};

struct MASTER_BOOT_RECORD
{
	struct PARTITION_TLB_ENTRY * tlbEntries[4];
};

struct MASTER_BOOT_RECORD * LoadBootRecord(void * bootRecordData);

#endif 