#include "rtos/gb_time.h"



void mdelay(unsigned int timeout)
{
	OS_ERR	err;
	
	OSTimeDly(M2T(timeout), OS_OPT_TIME_DLY, &err);
}

unsigned int sys_get_ms(void)
{
	OS_ERR	err;
	
	return (unsigned int)(T2M(OSTimeGet(&err)));
}

int timer_creat(timer_handle_t *os_timer, const char *name, unsigned int period, timer_fun timer, void *arg)
{
	OS_ERR	err;
	
	OSTmrCreate((OS_TMR            *)os_timer,
              (CPU_CHAR          *)name,
              (OS_TICK            )0,
              (OS_TICK            )TMR(period),
              (OS_OPT             )OS_OPT_TMR_PERIODIC,
              (OS_TMR_CALLBACK_PTR)timer,
              (void              *)arg,
              (OS_ERR            *)&err);
	if(err != OS_ERR_NONE)
		return -1;
	
	return 0;
}

int timer_start(timer_handle_t *os_timer)
{
	OS_ERR	err;
	if(OSTmrStart(os_timer, &err) == DEF_FALSE)
		return -1;
	
	return 0;
	
}

void timer_set_period(timer_handle_t *os_timer, unsigned int period)
{
	os_timer->Period = TMR(period);
}
