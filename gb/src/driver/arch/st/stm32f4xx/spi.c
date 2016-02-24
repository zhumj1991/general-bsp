#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gb_driver.h"
#include "gb_fifo.h"
#include "spi.h"
#include "gpio.h"

#define	SPI_TIMEOUT		50u
#define	SPI_FIFO_SIZE	16
#define SPI_DUMMY			0xA5

static void spi_func_init(struct driver * driver)
{
	SPI_InitTypeDef  SPI_InitStructure;
	SPI_TypeDef *SPIx = (SPI_TypeDef *)driver->base;
	struct spi_attr *attr = (struct spi_attr*)driver->attr;
	
	uint16_t scale = rcc_scale(driver->rcc, attr->baudrate);
	scale = (scale == 0)? 1 : scale;
	
	SPI_Cmd(SPIx, DISABLE);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = 8*(scale-1);
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIx, &SPI_InitStructure);
	SPI_Cmd(SPIx, ENABLE);
	
	SPI_I2S_ClearFlag(SPIx, SPI_I2S_FLAG_RXNE);
}

static void enable_cs(struct spi_attr *attr)
{
	struct gpio *chip_select = attr->chip_select;
	
	GPIO_ResetBits((GPIO_TypeDef*)chip_select->port, 1 << chip_select->pin);
}

static void disable_cs(struct spi_attr *attr)
{
	struct gpio *chip_select = attr->chip_select;
	
	GPIO_SetBits((GPIO_TypeDef*)chip_select->port, 1 << chip_select->pin);
}

static int8_t spi_init(struct driver * driver)
{
	struct spi_attr *attr = (struct spi_attr*)driver->attr;
	
	if(rcc_config(driver->rcc, (GB_StateType)ENABLE))
		return -1;
	
	spi_func_init(driver);
	
	if(sem_creat(driver->read_sem, 0) != 0)
		return -1;
	
	if(sem_creat(driver->write_sem, 0) != 0)
		return -1;
	
	if(mutex_creat(attr->xfer_mutex, 1) != 0)
		return -1;
	
	memset(attr->read_fifo, 0, sizeof(struct fifo));
	attr->read_fifo->size = SPI_FIFO_SIZE;
	attr->read_fifo->data = (uint8_t *)gb_malloc(SPI_FIFO_SIZE);
	
	register_interupt(attr->driver_irq, attr->irq ,attr->prio);
	
	return 0;
}

static int8_t spi_read(struct driver * driver, uint8_t * buf, uint8_t count)
{
	SPI_TypeDef *SPIx = (SPI_TypeDef *)driver->base;
	struct spi_attr *attr = (struct spi_attr*)driver->attr;
	
	mutex_lock(attr->xfer_mutex, SPI_TIMEOUT);
	SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
	enable_cs(attr);
	
	
	disable_cs(attr);
	SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, DISABLE);
	mutex_unlock(attr->xfer_mutex);
	
	return 0;
}

static int8_t spi_write(struct driver * driver, uint8_t* buf, uint8_t count)
{
	SPI_TypeDef *SPIx = (SPI_TypeDef *)driver->base;
	struct spi_attr *attr = (struct spi_attr*)driver->attr;	
	struct spi_transfer *xfer = (struct spi_transfer *)buf;
	uint8_t data = 0;
	
	if(count == 0)
		return -1;
	
	mutex_lock(attr->xfer_mutex, SPI_TIMEOUT);
	SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
	
	while(count--) {
		enable_cs(attr);
		
		uint8_t *p_tx = (uint8_t *)xfer->tx_buf;
		uint8_t *p_rx = (uint8_t *)xfer->rx_buf;
		uint8_t len = xfer->len;
		
		while(len--) {
			while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
			if(xfer->tx_buf != 0)
				SPI_I2S_SendData(SPIx, *p_tx++);
			else
				SPI_I2S_SendData(SPIx, SPI_DUMMY);

			if(sem_wait(driver->read_sem, SPI_TIMEOUT))
				return -1;
			fifo_read(attr->read_fifo, &data, 1);
			if(xfer->rx_buf != 0)
				*p_rx++ = data;
		}
		
		if(xfer->cs_change)
			disable_cs(attr);
		
		xfer++;
	}

	disable_cs(attr);
	SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, DISABLE);
	mutex_unlock(attr->xfer_mutex);
	
	return 0;
}

static int8_t spi_ioctl(struct driver * driver, uint8_t cmd, void * arg)
{
	SPI_TypeDef *SPIx = (SPI_TypeDef *)driver->base;
	struct spi_attr *attr = (struct spi_attr*)driver->attr;
	struct gpio *chip_select = attr->chip_select;
	struct spi_transfer *xfer;
	
	switch(cmd) {
		case 0:
			printf("Name\tInit?\tBaudrate\r\n");
			printf("%s\t%s\t%d\r\n", driver->name, (attr->init)?("ok"):("err"), attr->baudrate);

			break;
		case SPI_RDWD:
			xfer = (struct spi_transfer *)arg;
			uint8_t data = 0;
			uint8_t *p_tx = (uint8_t *)xfer->tx_buf;
			uint8_t *p_rx = (uint8_t *)xfer->rx_buf;
			uint8_t len = xfer->len;
		
			mutex_lock(attr->xfer_mutex, SPI_TIMEOUT);
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
		
			while(len--) {
				while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
				if(xfer->tx_buf != 0)
					SPI_I2S_SendData(SPIx, *p_tx++);
				else
					SPI_I2S_SendData(SPIx, SPI_DUMMY);

				if(sem_wait(driver->read_sem, SPI_TIMEOUT))
					return -1;
				fifo_read(attr->read_fifo, &data, 1);
				if(xfer->rx_buf != 0)
					*p_rx++ = data;
			}
			
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, DISABLE);
			mutex_unlock(attr->xfer_mutex);
			
			break;
		case SPI_CS:
			if(DISABLE == (int32_t)arg)
				disable_cs(attr);
			else
				enable_cs(attr);
			break;
		default:
			break;
	}
		
	return 0;
}

static int8_t spi_close(struct driver * driver)
{
	SPI_TypeDef *SPIx = (SPI_TypeDef *)driver->base;
	struct spi_attr *attr = (struct spi_attr*)driver->attr;
	
	gb_free(attr->read_fifo->data);
	
	sem_free(driver->read_sem);
	sem_free(driver->write_sem);
	
	mutex_free(attr->xfer_mutex);
		
	SPI_Cmd(SPIx, DISABLE);
	SPI_DeInit(SPIx);

	return 0;	
}
/*
 * spi flash variable and function
 */
// spi attr
static void spi_flash_isr(void);

static struct gpio cs_gpio = {
	.rcc				= {
				.bus 	= AHB1,
				.base = RCC_AHB1Periph_GPIOF
	},
	.port				= GPIOF_BASE,
	.pin				= GPIO_PinSource8
};

static struct fifo flash_read_fifo;
static mutex_handle_t spi_flash_transfer_mutex;

static struct spi_attr spi_flash_attr= {
	.init				= 0,
	
	.irq				= SPI3_IRQn,
	.prio				= 7,
	.driver_irq	= spi_flash_isr,
	
	.read_fifo	= &flash_read_fifo,
	
	.baudrate		= 10*1000*1000,
	.chip_select= &cs_gpio,
	.xfer_mutex	= &spi_flash_transfer_mutex,
};

// spi gpio
static struct rcc spi_gpio_rcc[] = {
	{
		.bus			= AHB1, 
		.base			= RCC_AHB1Periph_GPIOB
	},
};

int8_t spi_flash_gpio_init(struct driver* driver)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t i;
	
	for(i=0; i<(sizeof(spi_gpio_rcc)/sizeof(spi_gpio_rcc[0]));i++) {
		if(rcc_config(&spi_gpio_rcc[i], (GB_StateType)ENABLE))
			return -1;
	}
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
	

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	return 0;
}
// spi driver
sem_handle_t spi_flash_read_sem;
sem_handle_t spi_flash_write_sem;

static struct rcc spi_flash_rcc = {
	.bus				= APB1,
	.base				= RCC_APB1Periph_SPI3
};

static struct driver spi_flash __init_driver = {
	.name					= "spi_3",
	.base					= (uint32_t)SPI3_BASE,
	.rcc					= &spi_flash_rcc,
	
	.attr					= (void *)&spi_flash_attr,
	
	.read_sem			= &spi_flash_read_sem,
	.write_sem		= &spi_flash_write_sem,

	.gpio_init		= spi_flash_gpio_init,
	.func_init		= spi_init,
	.write				= spi_write,
	.ioctl				= spi_ioctl,
	.close				= spi_close
};

static void spi_flash_isr(void)
{
	SPI_TypeDef *SPIx = (SPI_TypeDef *)spi_flash.base;
	struct spi_attr *attr = (struct spi_attr*)spi_flash.attr;
	uint8_t temp = 0;
	
	RTOS_IRQ_PROTECT();
	
  if(SPI_GetITStatus(SPIx, SPI_I2S_IT_RXNE))
  {
    temp = SPI_I2S_ReceiveData(SPIx) & 0x00FF;
		fifo_write_byte(attr->read_fifo, temp);		
		sem_post(spi_flash.read_sem);
  }
	
	RTOS_IRQ_UNPROTECT();
}
