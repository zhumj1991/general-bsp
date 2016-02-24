#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gb_driver.h"
#include "gb_fifo.h"
#include "uart.h"

#define	UART_TIMEOUT		50u
#define UART_FIFO_SIZE	32u

static void uart_func_init(struct driver * driver)
{
	USART_InitTypeDef USART_InitStructure;
	USART_TypeDef *USARTx = (USART_TypeDef *)driver->base;
	struct uart_attr *attr = (struct uart_attr*)driver->attr;
	
	USART_Cmd(USARTx, DISABLE);
	
	USART_InitStructure.USART_BaudRate = attr->baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTx, &USART_InitStructure);
	
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	USART_Cmd(USARTx, ENABLE);

	USART_ClearFlag(USARTx, USART_FLAG_TC);
}


static int8_t uart_init(struct driver * driver)
{
	struct uart_attr *attr = (struct uart_attr*)driver->attr;
	
	if(rcc_config(driver->rcc, (GB_StateType)ENABLE))
		return -1;

	uart_func_init(driver);
	
	if(sem_creat(driver->read_sem, 0) != 0)
		return -1;
	
	if(sem_creat(driver->write_sem, 0) != 0)
		return -1;
	
	memset(attr->read_fifo, 0, sizeof(struct fifo));
	attr->read_fifo->size = UART_FIFO_SIZE;
	attr->read_fifo->data = (uint8_t *)gb_malloc(UART_FIFO_SIZE);

	memset(attr->write_fifo, 0, sizeof(struct fifo));
	attr->write_fifo->size = UART_FIFO_SIZE;
	attr->write_fifo->data = (uint8_t *)gb_malloc(UART_FIFO_SIZE);
	
	register_interupt(attr->driver_irq, attr->irq ,attr->prio);
	
	return 0;
}

static int8_t uart_read(struct driver * driver, uint8_t * buf, uint8_t count)
{
	struct uart_attr *attr = (struct uart_attr*)driver->attr;
	uint8_t len = 0, size = 0;

	while(count > 0 && 0 == sem_wait(driver->read_sem, UART_TIMEOUT)) {
		len = fifo_read(attr->read_fifo, buf, count);
		buf += len;
		count -= len;
		size += len;
	}
	
	return size;
}

static int8_t uart_write(struct driver * driver, uint8_t* buf, uint8_t count)
{
	USART_TypeDef *USARTx = (USART_TypeDef *)driver->base;
	struct uart_attr *attr = (struct uart_attr*)driver->attr;
	uint8_t temp;
	uint8_t len = 0, size = 0;
	
	do {
		len = fifo_write(attr->write_fifo, buf, count);
		buf += len;
		size += len;
		count -= len;
		
		if(fifo_read_byte(attr->write_fifo, &temp)) {	
			while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
			USART_SendData(USARTx, temp);			
			USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
		}
	} while(0 == sem_wait(driver->write_sem, UART_TIMEOUT) && count > 0);
	
	return size;
}

static int8_t uart_ioctl(struct driver * driver, uint8_t cmd, void * arg)
{
	struct uart_attr *attr = (struct uart_attr*)driver->attr;
	
	switch(cmd) {
		case 0:
			printf("Name\tInit?\tBaudrate\r\n");
			printf("%s\t%s\t%d\r\n", driver->name, (attr->init)?("ok"):("err"), attr->baudrate);

			break;
		case UART_BD_SET:
			attr->baudrate = *((uint32_t *)arg);
			uart_func_init(driver);
			break;
		case UART_BD_GET:
			*((uint32_t *)arg) = attr->baudrate;
			break;
		case 3:
			if(*((uint32_t *)arg) == 0)
				driver->close(driver);
			else
				driver->func_init(driver);
			break;
		default:
			break;
	}
		
	return 0;
}

static int8_t uart_close(struct driver * driver)
{
	USART_TypeDef *USARTx = (USART_TypeDef *)driver->base;
	struct uart_attr *attr = (struct uart_attr *)driver->attr;

	gb_free(attr->read_fifo->data);
	gb_free(attr->write_fifo->data);
		
	sem_free(driver->read_sem);
	sem_free(driver->write_sem);
	
	USART_Cmd(USARTx, DISABLE);
	USART_DeInit(USARTx);

	return 0;	
}
/*
 * console variable and function
 */
static struct fifo console_read_fifo;
static struct fifo console_write_fifo;

static void console_isr(void);

static struct uart_attr console_attr= {
		.init				= 0,
		.irq				= USART1_IRQn,
		.prio				= 5,
		.driver_irq	= console_isr,
		.read_fifo	= &console_read_fifo,
		.write_fifo	= &console_write_fifo,
		.baudrate		= CONSOLE_BAUDRATE
};


static struct rcc console_gpio_rcc[] = {
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOA
	},
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOA
	}
};

static struct rcc console_rcc = {
	.bus				= APB2,
	.base				= RCC_APB2Periph_USART1
};

int8_t console_gpio_init(struct driver* driver)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t i;
	
	for(i=0; i<(sizeof(console_gpio_rcc)/sizeof(console_gpio_rcc[0]));i++) {
		if(rcc_config(&console_gpio_rcc[i], (GB_StateType)ENABLE))
			return -1;
	}
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	return 0;
}

sem_handle_t console_read_mutex;
sem_handle_t console_write_mutex;

static struct driver console __init_driver = {
	.name					= "uart_1",
	.base					= (uint32_t)USART1_BASE,
	.rcc					= &console_rcc,
	
	.attr					= (void *)&console_attr,
	
	.read_sem		= &console_read_mutex,
	.write_sem	= &console_write_mutex,

	.gpio_init		= console_gpio_init,
	.func_init		= uart_init,
	.read 				= uart_read,
	.write				= uart_write,
	.ioctl				= uart_ioctl,
	.close				= uart_close
};

static void console_isr(void)
{
	USART_TypeDef *USARTx = (USART_TypeDef *)console.base;
	struct uart_attr *attr = (struct uart_attr*)console.attr;
	uint8_t temp = 0;
	
	RTOS_IRQ_PROTECT();

  if(USART_GetITStatus(USARTx, USART_IT_TXE))
  {
		 USART_ClearITPendingBit(USARTx, USART_IT_TXE);
		
		if(fifo_read_byte(attr->write_fifo, &temp)) {
			USART_SendData(USARTx, temp);
		} else {
			USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
			sem_post(console.write_sem);
		}
  }
	
  if(USART_GetITStatus(USARTx, USART_IT_RXNE))
  {
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		
    temp = USART_ReceiveData(USARTx) & 0x00FF;
		fifo_write_byte(attr->read_fifo, temp);
		sem_post(console.read_sem);
		
  }
	
	RTOS_IRQ_UNPROTECT();
}

/****************************************************************************************/

/*
 * printf
 */
int fputc(int ch, FILE *f)
{
	console.write(&console, (uint8_t *)&ch, 1);

	return ch;
}
