#include "os.h"
#include "rtos/gb_malloc.h"

void * gb_malloc(size_t size)
{
	return malloc(size);
}

void gb_free(void *ptr)
{
	free(ptr);
}
