#ifndef _VFS_H
#define _VFS_H

#include <stdint-gcc.h>

typedef uint32_t vfs_dirent;

typedef  vfs_dirent(*fs_open_dir)(char * path, void * driver);
typedef  char * (*fs_read_dir)(vfs_dirent *, void * driver);

typedef struct
{
	fs_open_dir fsopendir;
	fs_read_dir fsreaddir;
	void  * native_driver;
} fs_driver;

typedef struct
{
char * mountpath;
fs_driver * vfs_driver;
}vfs_mount_node;


char * splitpath(char * path, char delim);

int vfs_mount(fs_driver * fs);

vfs_dirent opendir(const char * path);


#endif