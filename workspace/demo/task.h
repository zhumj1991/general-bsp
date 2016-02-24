#ifndef __TASK_H
#define __TASK_H

#include "rtos/gb_system.h"

// Task enable
#define TASK_DIAG_ENABLE							1
#define TASK_LEDSEQ_ENABLE						1
#define TASK_ETH_ENABLE								1


#if (TASK_DIAG_ENABLE == 1)
	#include "command.h"
	
	#define TASK_DIAG_PRIO							(OS_CFG_PRIO_MAX - 3u)
	#define TASK_DIAG_STK_SIZE					400u
	
	void task_diag (void *p_arg);
	
#endif

#if TASK_LEDSEQ_ENABLE
	#include "led.h"
#endif

#if (TASK_ETH_ENABLE == 1)
	#include "dm9000x.h"
	
	#define TASK_ETH_PRIO								7
	#define TASK_ETH_STK_SIZE						128u
	
	void task_eth (void *p_arg);
	
	#define  LWIP_TASK_START_PRIO				4
	#define  LWIP_TASK_END_PRIO					6
	
#endif


void nothing_loop(void);

#endif
