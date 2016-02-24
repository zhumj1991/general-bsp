#ifndef __GB_SYSTEM_H
#define __GB_SYSTEM_H


typedef enum {
	RTOS_UCOS = 1,
	RTOS_FTEERTOS,
} SYS_TYPE;

#define	RTOS_TYPE			RTOS_UCOS

#if (RTOS_TYPE == RTOS_UCOS)
	#include "os.h"

	#define RTOS_IRQ_PROTECT()		do{		\
																	CPU_SR_ALLOC();				\
																	CPU_CRITICAL_ENTER();	\
																	OSIntNestingCtr++;		\
																	CPU_CRITICAL_EXIT();	\
																} while(0);
	
	#define RTOS_IRQ_UNPROTECT()	OSIntExit()
																	
#elif (RTOS_TYPE == RTOS_FTEERTOS)
	#include "freertos.h"
	#include "task.h"
#endif

#include "gb_malloc.h"
#include "gb_sem.h"
#include "gb_thread.h"
#include "gb_time.h"

void rtos_init(void);
void rtos_start(void);

#endif
