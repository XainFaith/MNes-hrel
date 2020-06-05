#ifndef _VFS_H
#define _VFS_H

#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>


typedef DIR *(*fs_open_dir)(char * path, void * driver); 
typedef dirent * (*fs_read_dir)(DIR *, void * driver);
typedef void (*fs_rewind_dir)(DIR *, void * driver);
typedef int (*fs_close_dir)(DIR *, void * driver);
typedef FILE * (*fs_open_file)(char * path, void * driver);
typedef int (*fs_read_file)(void *ptr, size_t size, void * file, void * driver);
typedef int (*fs_close_file)(void * file, void * driver);
typedef int (*fs_fstat)(void * file, void * driver, struct stat*);

typedef struct fs_driver
{
	void  * native_fs;
	fs_open_dir fsopendir;
	fs_close_dir fsclosedir;
	fs_rewind_dir fsrewinddir;
	fs_read_dir fsreaddir;
	fs_open_file fsopenfile;
	fs_read_file fsreadfile;
    fs_close_file fsclosefile;
    fs_fstat fsfstat;
} fs_driver;

typedef struct vfs_open_dir
{
	DIR * dir;
	fs_driver * driver;
} vfs_open_dir;

typedef struct vfs_open_file
{
	void * meta;
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
int vfsfopen(const char *, const char *);
int vfsfclose(FILE * stream);
size_t vfsfread(void *ptr, size_t size, size_t nmemb, FILE *stream);

int vfsfstat(FILE* stream, struct stat * pstat);

#endif
