#include <stdint.h>
#include <unistd.h>
#include <errno.h>


size_t _write(int fd, const char * buffer, size_t cnt)
{
	if(fd < 3)
	{
		for(int i =0; i < cnt; i++)
		{
			console_write(buffer[i]);
		}
	}
	
	return 0;
}

size_t _read(int fd, void *buf, size_t cnt)
{
	return cnt;
}

int _close(int fd)
{
	return 0;
}

int _fstat(int fd, struct stat *pstat)
{
	if(fd < 3) 
	{
		//pstat = S_IFCHR;
	}
	else
	{
		//pstat = S_IFBLK;
	}
	
	return 0;
}

int _isatty(int fd)
{
	errno = ENOTTY;
	return -1;
}

int _lseek(int fildes, off_t offset, int whence)
{
	return -1;
}

void * _sbrk(int incr)
{
	while(1){}
	return malloc(incr);
}


