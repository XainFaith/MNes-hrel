#ifndef _VFS_H
#define _VFS_H

#include <stdint-gcc.h>
#include <stdio.h>
#include <dirent.h>


typedef  DIR *(*fs_open_dir)(char * path, void * driver); 
typedef  dirent * (*fs_read_dir)(DIR *, void * driver);
typedef void (*fs_rewind_dir)(DIR *, void * driver);
typedef int (*fs_close_dir)(DIR *, void * driver);
typedef FILE * (*fs_open_file)(char * path, void * driver);
typedef int (*fs_read_file)(void *ptr, size_t size, void * file, void * driver);

typedef struct fs_driver
{
	void  * native_fs;
	fs_open_dir fsopendir;
	fs_close_dir fsclosedir;
	fs_rewind_dir fsrewinddir;
	fs_read_dir fsreaddir;
	fs_open_file fsopenfile;
	fs_read_file fsreadfile;
} fs_driver;

typedef struct vfs_open_dir
{
	DIR * dir;
	fs_driver * driver;
} vfs_open_dir;

typedef struct vfs_open_file
{
	void * data;
	void * meta;
	uint32_t pos;
	uint32_t size;
	char * mode;
	fs_driver * driver;
} vfs_open_file;

typedef struct vfs_mount_node
{
char * mountpath;
fs_driver * vfs_driver;

struct vfs_mount_node * prev;
struct vfs_mount_node * next;
}vfs_mount_node;

char * splitpath(char * path, char delim);

int vfs_mount(char * path , fs_driver * fs);

DIR * opendir(char * path);
int closedir(DIR * dir);
void rewinddir(DIR * dir);
dirent* readdir(DIR* dir);
FILE * fopen(const char *, const char *);

#endif