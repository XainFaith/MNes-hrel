#include <stdio.h>
#include "mbr.h"
#include "malloc.h"


struct PARTITION_TLB_ENTRY * pte_load(void * entryBuffer)
{
   struct PARTITION_TLB_ENTRY * pte = malloc(sizeof(struct PARTITION_TLB_ENTRY));
    uint8_t * readPtr = (uint8_t*)entryBuffer;
    //Skip attrb value
    readPtr++;
    //read in chs partition start address
    pte->paritionAddr = readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
    readPtr += 3;
    //Read in parition type
    pte->partitionType = *readPtr;
    readPtr++;
 
    //read in chs partition last address
    pte->lastPartAddr = readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
    readPtr += 3;
 
    //Read in LBA of partition Start
    pte->LBA = readPtr[3] << 24 | readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
    readPtr += 4;
 
    //Read in sector count
    pte->sectorCnt = readPtr[3] << 24 | readPtr[2] << 16 | readPtr[1] << 8 | readPtr[0];
    readPtr += 4;
 
    return pte;
}

struct MASTER_BOOT_RECORD * LoadBootRecord(void * bootRecordData)
 {
	 struct MASTER_BOOT_RECORD * mbr = (struct MASTER_BOOT_RECORD *)malloc(sizeof(struct MASTER_BOOT_RECORD));
    //Load in the Parition Tlb Entries Data
    uint8_t * recordData = (uint8_t*)bootRecordData;
    recordData += 0x1BE;
    mbr->tlbEntries[0] = pte_load(recordData);
    recordData += 16;

    mbr->tlbEntries[1]= pte_load(recordData);
    recordData += 16;

    mbr->tlbEntries[2] = pte_load(recordData);
    recordData += 16;
	
    mbr->tlbEntries[3]= pte_load(recordData);
    recordData += 16;
 
    uint16_t brCode = *recordData;
	return mbr;
 }