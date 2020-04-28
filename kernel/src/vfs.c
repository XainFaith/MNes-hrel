#include <stdint-gcc.h>

#include "filesystem.h"
#include "vfs.h"


struct VFS_NODE
{
	
};

struct VFS_MOUNT_NODE
{
	struct FILESYSTEM * fs;
	char * mname;
};

