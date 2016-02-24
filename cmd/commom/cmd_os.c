#include <stdio.h>
#include "stm32f4xx.h"
#include "command.h"
#include "os.h"

#if (CFG_COMMANDS & CFG_CMD_OS)
/***************************************************************************
 * DIAG_CMD	: ps
 * Help			: print ucos usage
 **************************************************************************/
static unsigned char usage_init = 0;
int 
do_ps (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if OS_CFG_DBG_EN > 0u
	OS_TCB	*p_tcb;			/* TCB = TASK CONTROL BLOCK */

	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
	p_tcb = OSTaskDbgListPtr;
	CPU_CRITICAL_EXIT();

	if(usage_init == 0) {
		OS_ERR err;	
		OSStatTaskCPUUsageInit(&err);
		mdelay(200);
		usage_init = 1;
	}
	
	printf(" Prio\tUsed\tFree\tPer\tUsage\tTaskname\r\n");

	while (p_tcb != (OS_TCB *)0) 
	{
		printf(" %02d\t%4d\t%4d\t%02d%%\t%05.2f%%\t%s\r\n", 
		p_tcb->Prio, 
		p_tcb->StkUsed, 
		p_tcb->StkFree, 
		(p_tcb->StkUsed * 100) / (p_tcb->StkUsed + p_tcb->StkFree),
		(float)p_tcb->CPUUsage / 100,
		p_tcb->NamePtr);		
	 	
		CPU_CRITICAL_ENTER();
		p_tcb = p_tcb->DbgNextPtr;
		CPU_CRITICAL_EXIT();
	}	
#endif
	
	return 0;
}

DIAG_CMD(
	ps, 1, 1, do_ps,
 	"ps      - print ucos usage\r\n",
	NULL
);

#endif

/***************************************************************************
 * DIAG_CMD	: reset
 * Help			: reset system
 **************************************************************************/
void reset_cpu(void)
{
	NVIC_SystemReset();
}

int 
do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	OS_ERR err;
	unsigned char delay = 5;

	printf("reset in %ds..", delay);
	while(delay--)
	{
		OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);
		printf("\b\b\b\b%ds..", delay);
	}
	serial_puts("\r\n\n");
	
	disable_interrupts();
	reset_cpu();
	
	return 0;
}

DIAG_CMD(
	reset, 1, 1, do_reset,
 	"reset   - perform RESET of the CPU\r\n",
	NULL
);

