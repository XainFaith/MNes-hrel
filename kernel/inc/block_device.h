#ifndef _BLOCK_DEVICE_H
#define _BLOCK_DEVICE_H

#include <stdint-gcc.h>
#include <stdbool.h>

typedef int (*BLOCK_READ)(void * buffer, uint32_t addr);
typedef int (*BLOCK_WRITE)(void * buffer, uint32_t addr);
typedef uint32_t (*GET_BLOCK_COUNT)();

struct BLOCK_DEVICE
{
	bool present;
	BLOCK_READ read;
	BLOCK_WRITE write;
	GET_BLOCK_COUNT getblockcount;
};

#endif