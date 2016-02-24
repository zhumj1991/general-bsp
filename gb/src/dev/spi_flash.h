#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include <stdint.h>


#define SF_MAX_PAGE_SIZE	(4 * 1024)


#define CMD_AAI       	0xAD  	/* AAI �������ָ��(FOR SST25VF016B) */



#define CMD_WREN      	0x06		/* дʹ������ */
#define CMD_EWRSR	  		0x50		/* ����д״̬�Ĵ��������� */
#define CMD_DISWR	  		0x04		/* ��ֹд, �˳�AAI״̬ */
#define CMD_RDSR      	0x05		/* ��״̬�Ĵ������� */
#define CMD_WRSR      	0x01  	/* д״̬�Ĵ������� */
#define CMD_READ      	0x03  	/* ������������ */
#define CMD_RDID      	0x9F		/* ������ID���� */
#define CMD_SE        	0x20		/* ������������ */
#define CMD_BE        	0xC7		/* ������������ */
#define CMD_POWER_DOWN	0xB9
#define	CMD_POWER_UP		0xAB
#define DUMMY_BYTE			0xA5		/* ���������Ϊ����ֵ�����ڶ����� */

#define WIP_FLAG				0x01		/* ״̬�Ĵ����е����ڱ�̱�־��WIP) */

/* ���崮��Flash ID */
enum
{
	SST25VF016B_ID	= 0x4125BF,//0xBF2541,
	MX25L1606E_ID		= 0x1520C2,//0xC22015
	W25Q64BV_ID			= 0x1740EF,//0xEF4017
	W25Q128FV_ID		= 0x1840EF,//0xEF4018
};

typedef struct
{
	uint32_t chipID;			/* оƬID */
	char chip_name[16];		/* оƬ�ͺ��ַ�������Ҫ������ʾ */
	uint32_t total_size;	/* ������ */
	uint16_t sector_size;
}spi_flash_info;



void spi_flash_read_buffer(uint8_t *buf, uint32_t read_addr, uint32_t size);
int8_t spi_flash_write_buffer(uint8_t *buf, uint32_t write_addr, uint16_t size);

void spi_flash_read_sector(uint8_t *buf, uint32_t read_addr, uint16_t size);
void spi_flash_write_sector(uint8_t *buf, uint32_t write_addr, uint16_t size);
void spi_flash_erase_sector(uint32_t sector_addr);
void spi_flash_erase_chip(void);

int spi_flash_init(void);

#endif
