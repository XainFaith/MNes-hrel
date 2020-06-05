#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>

//Initalizes Physical Memory Manager with 
int32_t init_pmm();

//Allocates a number of page frames
void * allocPage(uint32_t numOfPages);

//Frees  page frame chunk
void freePage(void * addr, uint32_t numOfPages);

#endif
