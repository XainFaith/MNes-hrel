#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "filesystem.h"
#include "vfs.h"
#include "dirent.h"

#define VFS_MAX_OPEN_DIR 512
#define VFS_MAX_OPEN_FILES 512

vfs_mount_node * vfs_mfs;

vfs_open_dir vfs_open_dirs[16];
vfs_open_file vfs_open_files[64];


int vfs_mount(char * path, fs_driver * fs)
{
	if(fs == NULL)
	{
		return -1;
	}
	//Only the first root fs can mount with a null path
	if(path == NULL && vfs_mfs != NULL)
	{
		return -1;
	}
	
	if(vfs_mfs == NULL)
	{
			vfs_mfs = (vfs_mount_node*)malloc(sizeof(vfs_mount_node));
			vfs_mfs->vfs_driver = fs;
			vfs_mfs->mountpath = "/";
			return 0;
	}
	
	//Check the current mounted systems for path collisions
	vfs_mount_node * itr = vfs_mfs;
	while(itr != NULL)
	{
		DIR* dir = itr->vfs_driver->fsopendir(path, itr->vfs_driver->native_fs);
		//If dir is not null then the mount path requested is in use and can not be made mountable
		if(dir != NULL)
		{
			return -1;
		}
		itr = itr->next;
	}
	
	//If we made it here then the mount path is valid
	vfs_mount_node * nmnt = (vfs_mount_node*)malloc(sizeof(vfs_mount_node));
	nmnt->vfs_driver = fs;
	nmnt->mountpath = path;
	
	itr = vfs_mfs;
	
	while(true)
	{
			if(itr->next != NULL)
			{
				itr = itr->next;
			}
			else
			{
				itr->next = nmnt;
				nmnt->prev = itr;
			}
	}
	
	return 0;
}

DIR *  opendir(char * path)
{
	int opendirindex = -1;
	for(int i =0; i < 16; i++)
	{
		if(vfs_open_dirs[i].dir == NULL)
		{
			opendirindex = i;
			break;
		}
	}
	
	if(opendirindex == -1) //No Space to open a directory to many open
	{
		return NULL;				//TODO set errno with a senseible value
	}
	
	if(path == NULL || vfs_mfs == NULL)
	{
		return NULL;
	}
	
	vfs_mount_node * itr = vfs_mfs;
	while(itr != NULL)
	{
		uint32_t pathlen = strlen(path);
		uint32_t mplen =  strlen(itr->mountpath);
		
		if(pathlen < mplen) //skip if the directory path is shorter then the mount point
		{
			itr = itr->next;
		}
		else
		{
			int cmpval  = strcmp( path,itr->mountpath);
			//if cmp == 0x0 root directory of the mount point is requested to be opened
			if(cmpval == 0x0)
			{
				DIR * dir = itr->vfs_driver->fsopendir("/", itr->vfs_driver->native_fs);
				if(dir != NULL)
				{
					vfs_open_dirs[opendirindex].dir = dir;
					vfs_open_dirs[opendirindex].driver = itr->vfs_driver;
					return dir;
				}
				
				return NULL;
			}
			else 
			{
				int mountpathlen = strlen(itr->mountpath);
				int cmpval = strncmp( path,itr->mountpath, mountpathlen);
				if(cmpval == 0x0) //requesting a sub directory of the mount point
				{
					char * subpath = path + strlen(itr->mountpath) ;
					DIR * dir = itr->vfs_driver->fsopendir(subpath,  itr->vfs_driver->native_fs);
					if(dir != NULL)
					{
						vfs_open_dirs[opendirindex].dir = dir;
						vfs_open_dirs[opendirindex].driver = itr->vfs_driver;
						return dir;
					}
					return NULL;
				}
			}
		}
			
		itr = itr->next;
	}

	return NULL;
}

dirent * readdir(DIR * dir)
{
	if(dir == NULL)
	{
		return NULL;
	}
	
	for(int i =0; i < 16; i++)
	{
		if(vfs_open_dirs[i].dir == dir)
		{
			return vfs_open_dirs[i].driver->fsreaddir(dir, vfs_open_dirs[i].driver->native_fs);
		}
	}
			
	return NULL;
}
	

int closedir(DIR * dir)
{
	if(dir == NULL)
	{
		return NULL;
	}

	for(int i =0; i < 16; i++)
	{
		if(vfs_open_dirs[i].dir == dir)
		{
			int retval = vfs_open_dirs[i].driver->fsclosedir(dir, vfs_open_dirs[i].driver->native_fs);
			//TODO check return value to be sure its sane before removing the open directory from the list of open directorys
			vfs_open_dirs[i].dir = NULL;
			vfs_open_dirs[i].driver = NULL;
		}
	}

}

void rewinddir(DIR * dir)
{
	if(dir == NULL)
	{
		return NULL;
	}
	
	for(int i =0; i < 16; i++)
	{
		if(vfs_open_dirs[i].dir == dir)
		{
			vfs_open_dirs[i].driver->fsrewinddir(dir, vfs_open_dirs[i].driver->native_fs);
		}
	}
}


FILE * fopen(const char * path, const char * mode)
{
	if(strcmp(mode,"r") != 0x0) {return NULL; } //Only read is going to be supported for now
	
	if(path == NULL || vfs_mfs == NULL)
	{
		return NULL;
	}

	vfs_mount_node * itr = vfs_mfs;
	while(itr != NULL)
	{
		uint32_t pathlen = strlen(path);
		uint32_t mplen =  strlen(itr->mountpath);
		
		if(pathlen < mplen)
		{
			itr = itr->next;
		}
		else
		{
			int cmpval  = strcmp( path,itr->mountpath);
			//if cmp == 0x0 root directory of the mount point is requested to be opened
			if(cmpval == 0x0)  //Cannot open a directory as a file
			{
				return NULL;
			}
			else
			{
				char * subpath = path + strlen(itr->mountpath) ;

			}
		}
		itr = itr->next;
	}
	
	return NULL; 
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
return 0;
}


