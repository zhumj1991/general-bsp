#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"

#if (CFG_SPI_FLASH)
	#include "spi_flash.h"
#endif

static uchar env_get_char_init (int32_t index);
uchar (*env_get_char)(int32_t) = env_get_char_init;

gd_t * gd;
/************************************************************************
 * Default settings to be used when no valid environment is found
 */
#define XMK_STR(x)	#x
#define MK_STR(x)		XMK_STR(x)

const uchar default_environment[] = {
#ifdef CONFIG_PASSWORD
	"password=" MK_STR(CONFIG_PASSWORD) "\0"
#endif	
#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	"bootdelay="	MK_STR(CONFIG_BOOTDELAY) "\0"
#endif
#if defined(CONFIG_BAUDRATE) && (CONFIG_BAUDRATE >= 0)
	"baudrate="	MK_STR(CONFIG_BAUDRATE) "\0"
#endif
#ifdef	CONFIG_LOADS_ECHO
	"loads_echo="	MK_STR(CONFIG_LOADS_ECHO) "\0"
#endif
#ifdef	CONFIG_ETHADDR
	"ethaddr="	MK_STR(CONFIG_ETHADDR) "\0"
#endif
#ifdef	CONFIG_IPADDR
	"ipaddr="	MK_STR(CONFIG_IPADDR) "\0"
#endif
#ifdef	CONFIG_SERVERIP
	"serverip="	MK_STR(CONFIG_SERVERIP) "\0"
#endif
#ifdef	CONFIG_ROOTPATH
	"rootpath="	MK_STR(CONFIG_ROOTPATH) "\0"
#endif
#ifdef	CONFIG_GATEWAYIP
	"gatewayip="	MK_STR(CONFIG_GATEWAYIP) "\0"
#endif
#ifdef	CONFIG_NETMASK
	"netmask="	MK_STR(CONFIG_NETMASK) "\0"
#endif
#ifdef	CONFIG_HOSTNAME
	"hostname="	MK_STR(CONFIG_HOSTNAME) "\0"
#endif
#ifdef	CONFIG_BOOTFILE
	"bootfile="	MK_STR(CONFIG_BOOTFILE) "\0"
#endif
#ifdef	CONFIG_LOADADDR
	"loadaddr="	MK_STR(CONFIG_LOADADDR) "\0"
#endif
	"\0"
};



static uchar env_get_char_init (int index)
{
	uint8_t c;

	/* if crc was bad, use the default environment */
	if (gd->env_valid)
	{
		c = *((uchar *)(gd->env_addr + index));
	} else {
//		printf("default_environment\r\n");
		c = default_environment[index];
	}

	return (c);
}

uchar *env_get_addr (int index)
{
	if (gd->env_valid) {
		return (((uchar *)(gd->env_addr + index)));
	} else {
		return (uchar *)(&default_environment[index]);
	}
}

void env_crc_update(void)
{

}	

int8_t saveenv(void)
{
	return !spi_flash_write_buffer((uchar *)gd->env_addr, 0, gd->env_size);
}

/*****************************************************************
 * Description: This function is called by your application 
 */
void gb_init(void)
{
	gd = (gd_t *)gb_malloc(sizeof(gd_t));
	gd->env_size = CFG_ENV_SIZE;
	gd->env_valid = 0;
	
#if (CFG_SPI_FLASH)
	spi_flash_init();
	#if (CFG_FILESYSTEM)
		if(gd->gb_filename != 0) {};
	#else
		gd->env_addr = (uint32_t)gb_malloc(gd->env_size);
		memset((void *)gd->env_addr, 0, gd->env_size);
		
		spi_flash_read_buffer((uchar *)gd->env_addr, 0, gd->env_size);
			
	#ifdef CONFIG_PASSWORD
		if(strncmp(((uchar *)gd->env_addr), "password", strlen("password")))
	#elif defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
		if(strncmp(((uchar *)gd->env_addr), "bootdelay", strlen("bootdelay")))
	#endif
		{
			memcpy((void *)gd->env_addr, &default_environment[0], sizeof(default_environment));
			if(spi_flash_write_buffer((uchar *)gd->env_addr, 0, gd->env_size))
				return;
		}
		gd->env_valid = 1;
	#endif
#else
	gd->env_addr = (uint32_t)&default_environment[0];
#endif
}
