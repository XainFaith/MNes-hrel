#include <stdint-gcc.h>
#include <stdio.h>
#include <dirent.h>
#include "pmm.h"
#include "pl110.h"
#include "pl181.h"
#include "console.h"
#include "vfs.h"
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
	fs_driver * fat_driver =  init_fatfs(sd);
	if(fat_driver != NULL)
	{
		int mval = vfs_mount(NULL,fat_driver);
		if(mval != 0)
		{
			printf("Failed to mount FS! \n");
			return;
		}
	}
	
	DIR * dir = opendir("/");
	
	dirent * entry = readdir(dir);
	while(entry != NULL)
	{
		printf("Entry: %s \n", entry->d_name);
		entry = readdir(dir);
	}
	
	rewinddir(dir);
	entry = readdir(dir);
	while(entry != NULL)
	{
		printf("Entry: %s \n", entry->d_name);
		entry = readdir(dir);
	}
	
	closedir(dir);
	
	entry = readdir(dir);
	while(entry != NULL)
	{
		printf("Entry: %s \n", entry->d_name);
		entry = readdir(dir);
	}

}