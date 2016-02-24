#include <stdio.h>
#include "stm32f4xx.h"
#include "gb_driver.h"
#include "rcc.h"
#include "gpio.h"


	
//int gpio_init(struct gpio *gpio)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//		
//	if(gpio->num <= 0)
//		return -1;
//	
//	for(unsigned char i=0; i<gpio->num; i++){
//		rcc_config(gpio->rcc + i, ENABLE);
//		
//		if(gpio->mode[i] == GPIO_Mode_AF && gpio->af[i] != 0) {
//			GPIO_PinAFConfig((GPIO_TypeDef *)gpio->port[i], gpio->pin[i], gpio->af[i]);
//		}
//		
//		GPIO_InitStructure.GPIO_OType = gpio->otype[i];
//		GPIO_InitStructure.GPIO_PuPd = gpio->pupd[i];
//		GPIO_InitStructure.GPIO_Mode = gpio->mode[i];
//		GPIO_InitStructure.GPIO_Pin = 1 << gpio->pin[i];
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//		GPIO_Init((GPIO_TypeDef *)gpio->port[i], &GPIO_InitStructure);
//	}
//	
//	return 0;
//}

int8_t gpio_init(struct driver* driver)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	struct gpio_attr * attr = (struct gpio_attr *)driver->attr;
	
	if(attr->num <= 0)
		return -1;

	for(uint8_t i=0; i<attr->num; i++){	
		if(rcc_config(&(attr->gpio+i)->rcc, (GB_StateType)ENABLE))
			return -1;
		
		GPIO_InitStructure.GPIO_Pin = 1 << ((attr->gpio + i)->pin);
		GPIO_InitStructure.GPIO_Mode = (GPIOMode_TypeDef)attr->dir;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		
		GPIO_Init((GPIO_TypeDef *)(attr->gpio+i)->port, &GPIO_InitStructure);
	}
	
	return 0;
}

int8_t gpio_ioctl(struct driver * driver, uint8_t cmd, void * arg)
{
	uint8_t i;
	struct gpio_attr * attr = (struct gpio_attr *)driver->attr;
	struct pin_value * pin;
	
	switch(cmd) {
		case 0:
			printf("Name\tInit?\tPort\tPin\tDir\tInput\tOutput\r\n%s", driver->name);
		
			for(i=0; i<attr->num; i++){
				printf("\t%s", (attr->init)?("ok"):("err"));
				printf("\t%c", PORT((attr->gpio + i)->port));
				printf("\t%d", (attr->gpio + i)->pin);
				printf("\t%s", (attr->dir == 0)? "in": "out");
				printf("\t%d", (GPIO_ReadInputData((GPIO_TypeDef*)(attr->gpio + i)->port) >> (attr->gpio + i)->pin) && 0x01);
				printf("\t%d\r\n", (GPIO_ReadOutputData((GPIO_TypeDef*)(attr->gpio + i)->port) >> (attr->gpio + i)->pin) && 0x01);
			}

			break;
		case GPIO_IN:
			pin = (struct pin_value *)arg;
			pin->value = (GPIO_ReadInputData((GPIO_TypeDef *)pin->port) >> (pin->pin)) && 0x01;
			break;
		case GPIO_OUT:
			pin = (struct pin_value *)arg;
			if(pin->value == 0)
				GPIO_ResetBits((GPIO_TypeDef*)pin->port, 1 << pin->pin);
			else
				GPIO_SetBits((GPIO_TypeDef*)pin->port, 1 << pin->pin);
			break;
		default:
			break;
	}
	
	return 0;
}

/*
 * GPIO input
 */
struct gpio pins_out[] = {
	[0] = {
			.rcc		= {
				.bus 	= AHB1,
				.base = RCC_AHB1Periph_GPIOI
			},
			.port			= GPIOI_BASE,
			.pin			= GPIO_PinSource10},
	[1] = {
			.rcc		= {
				.bus 	= AHB1,
				.base = RCC_AHB1Periph_GPIOF
			},
			.port			= GPIOF_BASE,
			.pin			= GPIO_PinSource7},
	[2] = {
			.rcc		= {
				.bus 	= AHB1,
				.base = RCC_AHB1Periph_GPIOF
			},
			.port		= GPIOF_BASE,
			.pin		= GPIO_PinSource8},
	[3] = {
			.rcc		= {
				.bus 	= AHB1,
				.base = RCC_AHB1Periph_GPIOC
			},
			.port			= GPIOC_BASE,
			.pin			= GPIO_PinSource2}
};

struct gpio_attr gpio_out_attr = {
	.init				= 0,
	.dir				= 1,
	.num				= sizeof(pins_out)/sizeof(pins_out[0]),
	.gpio				= &pins_out[0],
};

static struct driver gpio __init_driver = {
	.name					= "gpio",
	
	.attr					= (struct attr *)&gpio_out_attr,

	.gpio_init		= 0,
	.func_init		= gpio_init,

	.ioctl				= gpio_ioctl,
};
