#ifndef __GB_CONFIG_H
#define __GB_CONFIG_H

#include "gb_types.h"
#include "cmd_config.h"

typedef enum {
	STATE_ON = 0,
	STATE_OFF,
} GB_StateType;

#define DIAG_ENABLE
#ifdef DIAG_ENABLE
	#define diag_printf(fmt,args...)	debug(fmt,args...)
#else
	#define diag_printf(fmt,args...)
#endif

#define OS_UCOS
#include "os.h"


#ifdef STM32F40_41xxx
#include "stm32f4xx.h"

//rcc
#define AHB1		0x01
#define AHB2		0x02
#define	AHB3		0x03
#define APB1		0x11
#define APB2		0x12

#endif

//uart
#ifdef CONFIG_BAUDRATE
 #define CONSOLE_BAUDRATE			CONFIG_BAUDRATE
#else
 #define CONSOLE_BAUDRATE			115200
#endif




#endif
