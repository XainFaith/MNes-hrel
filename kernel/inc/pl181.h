#ifndef _PL181_H
#define _PL181_H

#include <stdint.h>
#include "block_device.h"

int init_pl181(uint32_t baseaddr);
struct BLOCK_DEVICE * pl181_get_block_device();

#endif
