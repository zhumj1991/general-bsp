#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include <stdint.h>

typedef void (*interrupt_handler)(void);


void interrupt_init(void);
int8_t register_interupt(interrupt_handler handler, uint8_t irq, uint8_t prio);

#endif
