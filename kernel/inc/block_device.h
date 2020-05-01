#ifndef _BLOCK_DEVICE_H
#define _BLOCK_DEVICE_H

#include <stdint-gcc.h>
#include <stdbool.h>

typedef int (*BLOCK_READ)(void * buffer, uint32_t addr);
typedef int (*BLOCK_WRITE)(void * buffer, uint32_t addr);

typedef int (*MULTI_BLOCK_READ)(void * buffer, uint32_t addr, uint32_t numblocks);
typedef int (*MULTI_BLOCK_WRITE)(void * buffer, uint32_t addr, uint32_t numblocks);

typedef uint32_t (*GET_BLOCK_COUNT)();

struct BLOCK_DEVICE
{
	bool present;
	BLOCK_READ read;
	BLOCK_WRITE write;
	MULTI_BLOCK_READ readblocks;
	MULTI_BLOCK_WRITE writeblocks;
	GET_BLOCK_COUNT getblockcount;
};

#endif