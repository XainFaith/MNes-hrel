#include <stdint-gcc.h>
#include <stdio.h>
#include "pmm.h"
#include "pl110.h"
#include "pl181.h"
#include "console.h"

#include "block_device.h"
#include "fat32.h"




void kmain()
{
	init_pmm();
	
	//Init lcd display and console
	init_pl110(0x10120000);
	init_console();
	
	//Initalize mmci
	init_pl181(0x10005000);
	struct BLOCK_DEVICE * sd = get_block_device();
	init_fatfs(sd);
	
}