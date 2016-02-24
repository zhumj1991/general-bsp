#ifndef __GB_THREAD
#define __GB_THREAD

typedef void (*thread_fun)(void *arg);

int thread_creat(thread_fun thread, void *arg, const char *name, int stacksize, int prio);
	
#endif
