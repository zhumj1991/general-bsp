#ifndef __GB_TIME_H
#define __GB_TIME_H

#include "gb_system.h"

#if (RTOS_TYPE == RTOS_UCOS)
	#define M2T(x)		((x*OSCfg_TickRate_Hz)/1000)
	#define T2M(x)		(x*(1000/OSCfg_TickRate_Hz))
	#define TMR(x)		(x/OSTmrUpdateCnt)
	
	typedef OS_TMR timer_handle_t;
	
#elif (RTOS_TYPE == RTOS_FREERTOS)

#endif

typedef void (*timer_fun)(timer_handle_t *p_tmr, void *arg);



void mdelay(unsigned int timeout);
#define msleep	mdelay

unsigned int sys_get_ms(void);

int timer_creat(timer_handle_t *os_timer, const char *name, unsigned int period, timer_fun timer, void *arg);
int timer_start(timer_handle_t *os_timer);
void timer_set_period(timer_handle_t *os_timer, unsigned int period);

#endif
