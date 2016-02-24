#include "os.h"

void rtos_init(void)
{
	OS_ERR  err;
	
	OSInit(&err);
	Mem_Init();
	OSSchedRoundRobinCfg(DEF_ENABLED, 8, &err);
}

void rtos_start(void)
{
	OS_ERR  err;
	
	OSStart(&err);
}
