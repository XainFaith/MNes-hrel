#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <stdint.h>
#include "block_device.h"

typedef char* (*FS_READ_DIR)(char *);


struct FILESYSTEM
{
	struct BLOCK_DEVICE * dev;
	FS_READ_DIR read_dir;
};


#endif
