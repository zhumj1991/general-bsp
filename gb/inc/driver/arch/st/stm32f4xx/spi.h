#ifndef __SPI_H
#define __SPI_H
#include <stdint.h>
#include "gb_driver.h"

#define SPI_RDWD		0x41
#define SPI_CS			0x42


struct spi_attr {
	uint8_t				init;
	
	uint8_t				irq;
	uint8_t				prio;
	interrupt_handler	driver_irq;
	
	struct fifo * read_fifo;
	struct fifo * write_fifo;
	
	uint32_t			baudrate;
	struct gpio *	chip_select;
	mutex_handle_t	*xfer_mutex;
};

struct spi_transfer {
	const void	*	tx_buf;
	void				*	rx_buf;
	uint16_t				len;
	uint8_t				cs_change : 1;
};


#endif
