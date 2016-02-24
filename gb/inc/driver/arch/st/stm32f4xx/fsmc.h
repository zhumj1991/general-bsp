#ifndef __FSMC_H
#define __FSMC_H

#include "gb_driver.h"

struct fsmc_attr {
	unsigned char init;
	unsigned char width;
	
	unsigned char irq;
	unsigned char prio;
	interrupt_handler	driver_irq;
	interrupt_handler device_irq;
	
	unsigned char	bank;
	unsigned int	addr;
};

#define SRAM_IRQ_REGISTER				0x31

#endif
