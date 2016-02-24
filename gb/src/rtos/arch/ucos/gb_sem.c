#include "rtos/gb_sem.h"

int mutex_creat(mutex_handle_t * mutex, unsigned char count)
{
	OS_ERR err;
	
	OSMutexCreate(mutex, "mutex", &err);
	if(err != OS_ERR_NONE)
		return -1;
	
	return 0;
}
int mutex_free(mutex_handle_t * mutex)
{
	OS_ERR err;
	
	OSMutexDel(mutex, OS_OPT_DEL_NO_PEND, &err);
	if(err == OS_ERR_NONE)
		return -1;
	
	return 0;
}

int mutex_lock(mutex_handle_t * mutex, unsigned int timeout)
{
	OS_ERR err;
	
	OSMutexPend(mutex, M2T(timeout), OS_OPT_PEND_BLOCKING,
					(CPU_TS *)0, &err);
	if(err != OS_ERR_NONE)
		return -1;
	return 0;
}

int mutex_unlock(mutex_handle_t * mutex)
{
	OS_ERR err;
	
	OSMutexPost(mutex, OS_OPT_POST_1, &err);
	if(err != OS_ERR_NONE)
		return -1;
	return 0;
}

int sem_creat(sem_handle_t * sem, unsigned char count)
{
	OS_ERR err;
	
	OSSemCreate(sem, "sem", count, &err);
	if(err != OS_ERR_NONE)
		return -1;
	
	return 0;
}
int sem_free(sem_handle_t * sem)
{
	OS_ERR err;
	
	OSSemDel(sem, OS_OPT_DEL_NO_PEND, &err);
	if(err == OS_ERR_NONE)
		return -1;
	
	return 0;
}

int sem_wait(sem_handle_t * sem, unsigned int timeout)
{
	OS_ERR err;
	
	OSSemPend(sem, M2T(timeout), OS_OPT_PEND_BLOCKING,
					(CPU_TS *)0, &err);
	if(err != OS_ERR_NONE)
		return -1;
	return 0;
}

int sem_post(sem_handle_t * sem)
{
	OS_ERR err;
	
	OSSemPost(sem, OS_OPT_POST_1, &err);
	if(err != OS_ERR_NONE)
		return -1;
	return 0;
}
