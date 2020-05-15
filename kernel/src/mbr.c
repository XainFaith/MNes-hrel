#include <stdio.h>
#include "mbr.h"
#include "malloc.h"


mbr_partion_tlb_entry * pte_load(void * entryBuffer)
{
    mbr_partion_tlb_entry * pte = (mbr_partion_tlb_entry*)malloc(sizeof(mbr_partion_tlb_entry));
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

master_boot_record  * LoadBootRecord(void * bootRecordData)
 {
	 master_boot_record  * mbr = (master_boot_record  *)malloc(sizeof(master_boot_record ));
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