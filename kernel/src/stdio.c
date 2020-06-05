#include <stdint.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "vfs.h"
#include "console.h"

size_t _write(int fd, const char * buffer, size_t cnt)
{
	if(fd < 3)
	{
		for(size_t i =0; i < cnt; i++)
		{
			console_write(buffer[i]);
		}
	}
	
	return 0;
}

size_t _read(int fd, void *buf, size_t cnt)
{
    if(fd < 3)
    {
    }
    else
    {
        return vfsfread(buf, cnt, 1, (FILE*)((uint32_t)fd));
    }
    
	return 0;
}

int _close(int fd)
{
    if(fd < 3)
    {
     return -1;   
    }
    else
    {
        return vfsfclose((FILE*)fd);
    }
	return 0;
}

int _fstat(int fd, struct stat *pstat)
{
	if(fd < 3) 
	{
		pstat = S_IFCHR;
	}
	else
	{
		pstat = S_IFBLK;
	}

	FILE* fileptr = (FILE*)fd;
    
	return vfsfstat(fileptr->_file, pstat);
}

int _isatty(int fd)
{
    if(fd){}
	errno = ENOTTY;
	return -1;
}

int _lseek(int fildes, off_t offset, int whence)
{
    if(offset){}
	return -1;
}

void * _sbrk(int incr)
{
    if(incr){}
	while(1){}
	return NULL;
}

int _open(const char *file, int flags, int mode)
{
    return vfsfopen(file,"r"); 
}

int putchar(int c)
{
 console_write((char)c);
 return c;
}

void _exit(int status)
{
}

void _kill()
{
}

int _getpid()
{
    return 0;
}
