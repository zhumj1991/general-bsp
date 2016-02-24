#include "os.h"
#include "rtos/gb_thread.h"
#include "rtos/gb_malloc.h"

int thread_creat(thread_fun thread, void *arg, const char *name, int stacksize, int prio)
{	
#if (OS_VERSION >= 30000u)
	OS_ERR		err;
	OS_TCB	*	task_tcb;
	CPU_STK * task_stk;
	
	task_tcb = (OS_TCB	*)gb_malloc(sizeof(OS_TCB));
	task_stk = (CPU_STK *)gb_malloc(sizeof(CPU_STK)*stacksize);
	
	OSTaskCreate((OS_TCB       *)task_tcb,
							 (CPU_CHAR  	 *)name,
							 (OS_TASK_PTR   )thread, 
							 (void         *)arg,
							 (OS_PRIO       )prio,
							 (CPU_STK      *)task_stk,
							 (CPU_STK_SIZE  )stacksize / 10,
							 (CPU_STK_SIZE  )stacksize,
							 (OS_MSG_QTY    )0,
							 (OS_TICK       )0,
							 (void         *)0,
							 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
							 (OS_ERR       *)&err);
#else
	OS_STK	*	task_stk;
	task_stk = (OS_STK *)gb_malloc(sizeof(OS_STK)*stacksize);
							 
	OSTaskCreate(thread, arg, task_stk, prio);
#endif

	return 0;
}
