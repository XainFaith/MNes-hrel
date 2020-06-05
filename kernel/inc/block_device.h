#ifndef _BLOCK_DEVICE_H
#define _BLOCK_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

typedef int (*block_read)(void * buffer, uint32_t addr);
typedef int (*block_write)(void * buffer, uint32_t addr);

typedef int (*multi_block_read)(void * buffer, uint32_t addr, uint32_t numblocks);
typedef int (*multi_block_write)(void * buffer, uint32_t addr, uint32_t numblocks);

typedef uint32_t (*get_block_count)();

typedef struct
{
	bool present;
	block_read read;
	block_write write;
	multi_block_read readblocks;
	multi_block_write writeblocks;
	get_block_count getblockcount;
}block_device;

#endif
