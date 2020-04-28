#include <string.h>
#include "errno.h"
#include "console.h"

/**
 * param fd the file descriptor to close
 * @return zero on success, or `-errno` on error
 */
int __sys_close(int fd)
{
	return 0;
}

/**
 * @param status the exit code
 */
_Noreturn void __sys_exit(int status)
{
}

/**
 * @param fd the output file descriptor
 * @param buffer the data buffer
 * @param count the byte count
 * @return the number of bytes written, or `-errno`
 */
long __sys_write(int fd, const void* buffer, unsigned long count)
{
	
	if(fd == 1)
	{
		char * cbuf = (char*)buffer;
		int len = strlen(cbuf);
		for(int i =0; i < len; i++)
		{
			console_write(cbuf[i]);
		}
	}
	
	return errno;
}