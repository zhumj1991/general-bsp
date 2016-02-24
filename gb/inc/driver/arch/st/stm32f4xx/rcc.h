#ifndef __RCC_H
#define __RCC_H

#include <stdint.h>
#include "gb_driver.h"


struct rcc {
	unsigned char bus;
	unsigned int base;
};

int8_t rcc_config(struct rcc *rcc, GB_StateType NewState);
uint8_t rcc_scale(struct rcc *rcc, uint32_t freq);

#endif
