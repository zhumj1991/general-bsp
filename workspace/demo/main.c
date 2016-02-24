#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "app_cfg.h"
#include "stm32f4xx.h"
#include "gb_bsp.h"
#include "gb_driver.h"
#include "gb_dev.h"
#include "rtos/gb_system.h"
#include "task.h"


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/  
#define TASK_START_PRIO								5u
#define TASK_START_STK_SIZE						256u


OS_MEM *driver_partition;
OS_MEM *dev_partition;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static void task_start (void * p_arg);

int main (void)
{
	rtos_init();
	thread_creat(task_start, 0, "Task Start", TASK_START_STK_SIZE, TASK_START_PRIO);
	rtos_start();
	return (0);
}

static void task_start (void *p_arg)
{
	CPU_Init();
	BSP_Tick_Init();	
	
	interrupt_init();
	driver_init();		
	gb_init();
	
#if (TASK_LEDSEQ_ENABLE)
	if(led_init() != 0) {
		while(1) mdelay(1000);
	}
	
	ledseq_init();
	ledseqRun(LED_RED, seq_alive);
#endif

#if (TASK_ETH_ENABLE)
	if(dm9k_init() != 0) {
#if (TASK_LEDSEQ_ENABLE)
	ledseqRun(LED_GREEN, seq_NotPassed);
#endif		
		while(1) mdelay(1000);
	}
	thread_creat(task_eth, 0, "Task Eth", TASK_ETH_STK_SIZE, TASK_ETH_PRIO);
#endif
							 
#if (TASK_DIAG_ENABLE)
	if(console_init() != 0) {
#if (TASK_LEDSEQ_ENABLE)
	ledseqRun(LED_GREEN, seq_NotPassed);
#endif	
		while(1) mdelay(1000);
	}
	thread_creat(task_diag, 0, "Task Diag", TASK_DIAG_STK_SIZE, TASK_DIAG_PRIO);
#endif
							 
	while(1) {
		mdelay(1000);
	}
}
