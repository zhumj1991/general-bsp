#ifndef __UART_H
#define __UART_H

#include "interrupt.h"

#define UART_BD_SET				0x20
#define UART_BD_GET				0x21

struct uart_attr {
	unsigned char	init;
	unsigned char irq;
	unsigned char prio;
	interrupt_handler	driver_irq;
	struct fifo * read_fifo;
	struct fifo * write_fifo;
	unsigned int baudrate;
};

#endif
