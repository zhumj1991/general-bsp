#include <stdio.h>
#include <string.h>
#include "gb_driver.h"
#include "gb_dev.h"
#include "spi.h"
#include "spi_flash.h"


static spi_flash_info flash_info;
static struct dev spi_flash_dev;
static uint8_t spi_buf[4 * 1024];	/* 用于写函数，先读出整个sector，修改缓冲区后，再整个page回写 */
static uint8_t dummy	= DUMMY_BYTE;

static int8_t spi_flash_auto_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size);
void spi_flash_read_sector(uint8_t *buf, uint32_t read_addr, uint16_t size);
void spi_flash_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size);
void spi_flash_erase_sector(uint32_t sector_addr);
static uint8_t spi_flash_send_byte(uint8_t value);


uint32_t spi_flash_read_chipID(void)
{
	uint8_t cmd = CMD_RDID;
	uint32_t id	= 0;
	
	struct spi_transfer t[] = {
		{
			.tx_buf			= &cmd,
			.rx_buf			= 0,
			.len				= 1,
			.cs_change	= 0,
		}, {
			.tx_buf			= 0,
			.rx_buf			= &id,
			.len				= 3,	
		}
	};
	
	spi_flash_dev.driver->write(spi_flash_dev.driver, (uint8_t *)t, 2);
	
	return id;
}

void spi_flash_read_info(void)
{

	flash_info.chipID = spi_flash_read_chipID();	/* 芯片ID */

	switch (flash_info.chipID)
	{
		case SST25VF016B_ID:
			strcpy(flash_info.chip_name, "SST25VF016B");
			flash_info.total_size = 2 * 1024 * 1024;	/* 总容量 = 2M */
			flash_info.sector_size = 4 * 1024;				/* 页面大小 = 4K */
			break;

		case MX25L1606E_ID:
			strcpy(flash_info.chip_name, "MX25L1606E");
			flash_info.total_size = 2 * 1024 * 1024;	/* 总容量 = 2M */
			flash_info.sector_size = 4 * 1024;				/* 页面大小 = 4K */
			break;

		case W25Q64BV_ID:
			strcpy(flash_info.chip_name, "W25Q64BV");
			flash_info.total_size = 8 * 1024 * 1024;	/* 总容量 = 8M */
			flash_info.sector_size = 4 * 1024;
			break;
		
		case W25Q128FV_ID:
			strcpy(flash_info.chip_name, "W25Q128FV");
			flash_info.total_size = 16 * 1024 * 1024;	/* 总容量 = 16M */
			flash_info.sector_size = 4 * 1024;
			break;

		default:
			strcpy(flash_info.chip_name, "Unknow Flash");
			flash_info.total_size = 2 * 1024 * 1024;
			flash_info.sector_size = 4 * 1024;
			break;
	}
}

static uint32_t spi_flash_wait_ready(void)
{
	uint32_t timeout = 1000; 
	
	spi_flash_send_byte(CMD_RDSR);
	while(((spi_flash_send_byte(DUMMY_BYTE) & WIP_FLAG) == SET) && (timeout--))	{ /* 判断状态寄存器的忙标志位 */
		mdelay(1);
	}
	
	return timeout;
}

static void spi_flash_write_status(uint8_t value)
{
	if (flash_info.chipID == SST25VF016B_ID)
	{
		spi_flash_send_byte(CMD_EWRSR);	/* 发送命令， 允许写状态寄存器 */
	}

	uint8_t	 data[2];
	
	struct spi_transfer t[] = {
		{
			.tx_buf			= &data[0],
			.rx_buf			= 0,
			.len				= 2,
		}
	};
	data[0] = CMD_WRSR;
	data[1] = value;
	
	spi_flash_dev.driver->write(spi_flash_dev.driver, (uint8_t *)t, 1);
}

static void spi_flash_write_enable(void)
{
	spi_flash_send_byte(CMD_WREN);
}

static int8_t spi_flash_cmp_data(uint32_t src_addr, uint8_t *tar, uint32_t size)
{
	uint8_t cmd = CMD_READ;
	uint32_t addr = (src_addr & 0xFF0000) >> 16 | (src_addr & 0xFF00) | (src_addr & 0xFF) << 16;
	uint8_t value = 0;

	if ((src_addr + size) > flash_info.total_size) return -1;
	if (size == 0) return -1;

	struct spi_transfer t[] = {
		{
			.tx_buf			= &cmd,
			.rx_buf			= 0,
			.len				= 1,
		}, {
			.tx_buf			= &addr,
			.rx_buf			= 0,
			.len				= 3,		
		}, {
			.tx_buf			= 0,
			.rx_buf			= &value,
			.len				= 1,
		}
	};
	
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)ENABLE);
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[0]);
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[1]);
	
	while (size--)
	{
		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[2]);
		if (*tar++ != value)
		{
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)DISABLE);
			return -1;
		}
	}
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)DISABLE);
	return 0;
}

void spi_flash_read_buffer(uint8_t *buf, uint32_t read_addr, uint32_t size)
{
	uint8_t cmd = CMD_READ;
	uint32_t addr	= (read_addr & 0xFF0000) >> 16 | (read_addr & 0xFF00) | (read_addr & 0xFF) << 16;
	uint8_t value = 0;
	
	struct spi_transfer t[] = {
		{
			.tx_buf			= &cmd,
			.rx_buf			= 0,
			.len				= 1,
		}, {
			.tx_buf			= &addr,
			.rx_buf			= 0,
			.len				= 3,
		}, {
			.tx_buf			= 0,
			.rx_buf			= &value,
			.len				= 1,
		}
	};
	
	if ((size == 0) ||(read_addr + size) > flash_info.total_size)
		return;

	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)ENABLE);
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[0]);
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[1]);
	
	while (size--) {
		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[2]);
		*buf++ = value;
	}
	
	spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)DISABLE);
}

int8_t spi_flash_write_buffer(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint16_t NumOfSector = 0, NumOfSingle = 0, add = 0, count = 0, temp = 0;

	add = write_addr % flash_info.sector_size;
	count = flash_info.sector_size - add;
	NumOfSector =  size / flash_info.sector_size;
	NumOfSingle = size % flash_info.sector_size;

	if (add == 0) /* 起始地址是页面首地址  */
	{
		if (NumOfSector == 0) /* 数据长度小于页面大小 */
		{
			if (spi_flash_auto_write_sector(buf, write_addr, size))
				return -1;
		}
		else 	/* 数据长度大于等于页面大小 */
		{
			while (NumOfSector--)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, flash_info.sector_size))
					return -1;

				write_addr +=  flash_info.sector_size;
				buf += flash_info.sector_size;
			}
			if (spi_flash_auto_write_sector(buf, write_addr, NumOfSingle))
				return -1;
		}
	}
	else  /* 起始地址不是页面首地址  */
	{
		if (NumOfSector == 0) /* 数据长度小于页面大小 */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				if (spi_flash_auto_write_sector(buf, write_addr, count))
					return -1;

				write_addr +=  count;
				buf += count;

				if (spi_flash_auto_write_sector(buf, write_addr, temp))
					return -1;
			}
			else
			{
				if (spi_flash_auto_write_sector(buf, write_addr, size))
					return -1;
			}
		}
		else	/* 数据长度大于等于页面大小 */
		{
			size -= count;
			NumOfSector =  size / flash_info.sector_size;
			NumOfSingle = size % flash_info.sector_size;

			if (spi_flash_auto_write_sector(buf, write_addr, count))
				return -1;

			write_addr +=  count;
			buf += count;

			while (NumOfSector--)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, flash_info.sector_size))
					return -1;
				
				write_addr +=  flash_info.sector_size;
				buf += flash_info.sector_size;
			}

			if (NumOfSingle != 0)
			{
				if (spi_flash_auto_write_sector(buf, write_addr, NumOfSingle))
					return -1;
			}
		}
	}
	
	return 0;
}

static uint8_t spi_flash_need_erase(uint8_t *old_buf, uint8_t *new_buf, uint16_t len)
{
	uint16_t i;
	uint8_t old;

	/*
	算法第1步：old 求反, new 不变
	    old    new
		  1101   0101
	~   1111
		= 0010   0101

	算法第2步: old 求反的结果与 new 位与
		  0010   old
	&	  0101   new
		 =0000

	算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
	*/

	for (i = 0; i < len; i++)
	{
		old = *old_buf++;
		old = ~old;

		if ((old & (*new_buf++)) != 0)
			return 1;
	}
	return 0;
}

static int8_t spi_flash_auto_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint16_t i;
	uint16_t j;					/* 用于延时 */
	uint32_t first_addr;	/* 扇区首址 */
	uint8_t need_erase;		/* 1表示需要擦除 */
	int8_t ret;

	if (size == 0) return -1;
	if (write_addr >= flash_info.total_size) return -1;
	if (size > flash_info.sector_size) return -1;

	/* 如果FLASH中的数据没有变化,则不写FLASH */
	spi_flash_read_buffer(spi_buf, write_addr, size);
	if (memcmp(spi_buf, buf, size) == 0)
		return 0;

	/* 判断是否需要先擦除扇区 */
	/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
	need_erase = 0;
	if (spi_flash_need_erase(spi_buf, buf, size))
		need_erase = 1;

	first_addr = write_addr & (~(flash_info.sector_size - 1));

	if (size == flash_info.sector_size)		/* 整个扇区都改写 */
	{
		for	(i = 0; i < flash_info.sector_size; i++)
		{
			spi_buf[i] = buf[i];
		}
	}
	else						/* 改写部分数据 */
	{
		/* 先将整个扇区的数据读出 */
		spi_flash_read_buffer(spi_buf, first_addr, flash_info.sector_size);

		/* 再用新数据覆盖 */
		i = write_addr & (flash_info.sector_size - 1);
		memcpy(&spi_buf[i], buf, size);
	}

	/* 写完之后进行校验，如果不正确则重写，最多3次 */
	ret = -1;
	for (i = 0; i < 3; i++)
	{

		/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
		if (need_erase == 1)
		{
			spi_flash_erase_sector(first_addr);		/* 擦除1个扇区 */
		}

		/* 编程一个PAGE */
		spi_flash_write_sector(spi_buf, first_addr, flash_info.sector_size);

		if (spi_flash_cmp_data(write_addr, buf, size) == 0)
		{
			ret = 0;
			break;
		}
		else
		{
			/* 失败后延迟一段时间再重试 */
			for (j = 0; j < 1000; j++);
		}
	}

	return ret;
}

// sector
void spi_flash_read_sector(uint8_t *buf, uint32_t read_addr, uint16_t size)
{
	uint8_t cmd = CMD_READ;
	uint32_t addr	= (read_addr & 0xFF0000) >> 16 | (read_addr & 0xFF00) | (read_addr & 0xFF) << 16;

	struct spi_transfer t[] = {
		{
			.tx_buf			= &cmd,
			.rx_buf			= 0,
			.len				= 1,
			.cs_change	= 0,
		}, {
			.tx_buf			= &addr,
			.rx_buf			= 0,
			.len				= 3,
			.cs_change	= 0,
		}, {
			.tx_buf			= 0,
			.rx_buf			= buf,
			.len				= size,
		}
	};

	spi_flash_dev.driver->write(spi_flash_dev.driver, (uint8_t *)t, 3);
}

void spi_flash_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size)
{
	uint32_t i, j;
	uint8_t cmd = CMD_AAI;
	uint32_t addr	= (write_addr & 0xFF0000) >> 16 | (write_addr & 0xFF00) | (write_addr & 0xFF) << 16;
	
	struct spi_transfer t[] = {
		{
			.tx_buf			= &cmd,
			.rx_buf			= 0,
			.len				= 1,
			.cs_change	= 0,
		}, {
			.tx_buf			= &addr,
			.rx_buf			= 0,
			.len				= 3,
			.cs_change	= 0,
		}, {
			.tx_buf			= buf,
			.rx_buf			= 0,
			.len				= 2,
		}
	};	
	
	if (flash_info.chipID == SST25VF016B_ID)
	{
		/* AAI指令要求传入的数据个数是偶数 */
		if ((size<2) && (size%2))
			return ;

		spi_flash_write_enable();			/* 发送写使能命令 */

		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)ENABLE);
	
		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[0]);
		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[1]);
		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[2]);
		buf += 2;

		spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)DISABLE);

		spi_flash_wait_ready();	/* 等待串行Flash内部写操作完成 */

		size -= 2;										/* 计算剩余字节数 */

		for (i = 0; i<(size/2); i++)
		{
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)ENABLE);
			
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[0]);
			t[2].tx_buf = buf;
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[2]);
			buf += 2;
			
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)DISABLE);
			
			spi_flash_wait_ready();/* 等待串行Flash内部写操作完成 */
		}
	}
	else	/* for MX25L1606E 、 W25Q64BV */
	{
		cmd = 0x02;
		
		for (j=0; j<(size/256); j++)
		{
			spi_flash_write_enable();								/* 发送写使能命令 */
			
			addr	= (write_addr & 0xFF0000) >> 16 | (write_addr & 0xFF00) | (write_addr & 0xFF) << 16;
			t[1].tx_buf = &addr;

			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)ENABLE);
	
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[0]);
			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[1]);

			t[2].len = 1;
			for (i=0; i<256; i++)
			{
				t[2].tx_buf = buf++;
				spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_RDWD, (void *)&t[2]);
			}

			spi_flash_dev.driver->ioctl(spi_flash_dev.driver, SPI_CS, (void *)DISABLE);

			spi_flash_wait_ready();

			write_addr += 256;
		}
	}
	
	spi_flash_send_byte(CMD_DISWR);
	spi_flash_wait_ready();	/* 等待串行Flash内部写操作完成 */
}

void spi_flash_erase_sector(uint32_t sector_addr)
{
	uint8_t cmd = CMD_SE;
	uint32_t addr	= (sector_addr & 0xFF0000) >> 16 | (sector_addr & 0xFF00) | (sector_addr & 0xFF) << 16;
	
	struct spi_transfer t[] = {
		{
			.tx_buf			= &cmd,
			.rx_buf			= 0,
			.len				= 1,
			.cs_change	= 0,
		}, {
			.tx_buf			= &addr,
			.rx_buf			= 0,
			.len				= 3,	
		}
	};
	
	spi_flash_write_enable();				/* 发送写使能命令 */
	spi_flash_dev.driver->write(spi_flash_dev.driver, (uint8_t *)t, 2);
	spi_flash_wait_ready();					/* 等待串行Flash内部写操作完成 */
}

void spi_flash_erase_chip(void)
{
	spi_flash_write_enable();				/* 发送写使能命令 */
	spi_flash_send_byte(CMD_BE);		/* 发送整片擦除命令 */
	spi_flash_wait_ready();					/* 等待串行Flash内部写操作完成 */
}

/*
 * spi flash device
 */

static struct dev spi_flash_dev = {
	.name = "flash",
	.driver_name = "spi_3",
};

int spi_flash_init(void)
{
	if(dev_mach(&spi_flash_dev) == 0) {
		spi_flash_read_info();
		if(spi_flash_wait_ready() == 0) {
			printf("\r\n***ERROR: no spi flash timeout!!\r\n");
			return -1;
		}
		spi_flash_write_status(0);
		
		printf("[flash_init]%s sector: %dB, size: %dB\r\n", 
				flash_info.chip_name, flash_info.sector_size, flash_info.total_size);
		
		return 0;
	}

	printf("\r\n***ERROR: no spi driver found!!\r\n");
	return -1;
}

static uint8_t spi_flash_send_byte(uint8_t value)
{
	uint8_t	data;
	
	struct spi_transfer t[] = {
		{
			.tx_buf			= &value,
			.rx_buf			= &data,
			.len				= 1,
		}
	};
	
	spi_flash_dev.driver->write(spi_flash_dev.driver, (uint8_t *)t, 1);
	
	return data;
}
