#include <stddef.h>
#include <stdint-gcc.h>
#include <pmm.h>
#include <liballoc.h>

//just stubs as there is no threading implmented right now anyways //TODO implment locking if threading is implmented
int liballoc_lock()
{
	return 0;
}

//just stubs as there is no threading implmented right now anyways //TODO implment locking if threading is implmented
int liballoc_unlock()
{
	return 0;
}

void* liballoc_alloc( int pages )
{
	void * allocatedPages = allocPage(pages);
	return allocatedPages;
}

int liballoc_free( void* ptr, int pages )
{
	freePage(ptr, pages);
	return 0;
}
