#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f4xx.h"
#include "rcc.h"

#define GPIO_IN						0x10
#define GPIO_OUT					0x11

#define PORT(x)						((char)((x - AHB1PERIPH_BASE)/0x0400 + 'A'))

struct gpio {
	struct	rcc		rcc;
	uint32_t			port;
	uint8_t				pin;
};

struct gpio_attr {
	uint8_t				init;
	uint8_t				dir;
	uint8_t				num;
	struct	gpio *gpio;
};

struct pin_value {
	uint32_t			port;
	uint8_t				pin;
	uint8_t				value;
};


//struct gpio {
//	unsigned char num;
//	
//	struct rcc *rcc;
//	const unsigned int *port;
//	const unsigned int *pin;
//	const unsigned char *otype;
//	const unsigned char *pupd;
//	const unsigned char *mode;
//	const unsigned char *af;
//};

//int gpio_init(struct gpio *gpio);

#endif
