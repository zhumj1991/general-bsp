#ifndef __GB_SEM_H
#define __GB_SEM_H

#include "gb_system.h"


#if (RTOS_TYPE == RTOS_UCOS)
	typedef	OS_MUTEX	mutex_handle_t;
	typedef	OS_SEM		sem_handle_t;
#elif (RTOS_TYPE == RTOS_FTEERTOS)

#endif



int mutex_creat(mutex_handle_t * mutex, unsigned char count);
int mutex_free(mutex_handle_t * mutex);
int mutex_lock(mutex_handle_t * mutex, unsigned int timeout);
int mutex_unlock(mutex_handle_t * mutex);

int sem_creat(sem_handle_t * sem, unsigned char count);
int sem_free(sem_handle_t * sem);
int sem_wait(sem_handle_t * sem, unsigned int timeout);
int sem_post(sem_handle_t * sem);

#endif
