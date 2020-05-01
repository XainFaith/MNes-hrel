#include <stdint-gcc.h>
#include <string.h>
#include "filesystem.h"
#include "vfs.h"

char * pathref;
char * pathreforigin;

char * splitpath(char * path, char delim)
{
	static int pstrsize =0 ;
	static int pstrprocessed = 0;
	
	if(path != NULL &&  delim != 0x0 &&  pathref == NULL)
	{
		pstrsize = strlen(path);
		pstrprocessed = 0;
		pathref = (char*)malloc(pstrsize);
		pathreforigin = pathref;
		memcpy(pathref, path, pstrsize);
		
		for(int i = 0; i < pstrsize; i++)
		{
			if(pathref[i] == delim)
			{
				pathref[i] = '\0';
			}
		}
		
		if(pathref[0] == '\0')
		{
			pathref++;
			pstrprocessed++;
		}

		return pathref;
	}
	
	if(path == NULL && pathref != NULL)
	{
			int offset = strlen(pathref);
			pstrprocessed += offset;

			if(pstrprocessed == pstrsize)
			{
					free(pathreforigin);
					pathref = NULL;
					pathreforigin = NULL;
					pstrprocessed = 0;
					pstrsize = 0;
					return NULL;
			}
	
			pathref += offset;
			
			//This edge case happens if there is a trailing delim
			if(strcmp(pathref, "") == 0)
			{
					free(pathreforigin);
					pathref = NULL;
					pathreforigin = NULL;
					pstrprocessed = 0;
					pstrsize = 0;
			}
			
			return pathref;
	}
	
	
	return NULL;
}


vfs_mount_node * vfs_mfs;

int vfs_mount(fs_driver * fs)
{
	if(vfs_mfs == NULL)
	{
			vfs_mfs = (vfs_mount_node*)malloc(sizeof(vfs_mount_node));
			vfs_mfs->vfs_driver = fs;
			return 0;
	}
	
	return -1;
}

vfs_dirent opendir(const char * path)
{
	if(vfs_mfs != NULL)
	{
		return vfs_mfs->vfs_driver->fsopendir(path, vfs_mfs->vfs_driver->native_driver);
	}
	
	return NULL;
}