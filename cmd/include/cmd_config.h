#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdint.h>
#include "rtos/gb_system.h"

typedef unsigned long ulong;
typedef unsigned char uchar;


#define CONFIG_DEBUG
#ifdef CONFIG_DEBUG
	#define debug(fmt,args...)	printf (fmt ,##args) 
#else
	#define debug(fmt,args...)
#endif

#define	ENV_SIZE							512


#define	CFG_CBSIZE						64			/* Console I/O Buffer Size */
#define	CFG_MAXARGS						16			/* Max number of command args	*/
#define CFG_LONGHELP
#define CFG_PROMPT						"#"
#define CFG_MEMTEST_START			0x20000000
#define CFG_MEMTEST_END				0x20020000

#define CFG_DM9000_BASE				0x68400000

#define CFG_COMMANDS					1
#define CFG_CMD_DATE					1
#define	CFG_CMD_LOAD					1
#define CFG_CMD_LOADB					1
#define	CFG_CMD_MMC						0
#define CFG_CMD_MEMORY				0
#define CFG_CMD_OS						1
#define	CFG_SPI_FLASH					1
#define CFG_ENV_SIZE					ENV_SIZE

#define CONFIG_PASSWORD				123456
#define CONFIG_BOOTDELAY			10
#define CONFIG_BAUDRATE				115200
#define CONFIG_ETHADDR				00:60:6e:90:00:ae
#define CONFIG_IPADDR					192.168.1.100
#define CONFIG_SERVERIP				192.168.1.101

#define CONFIG_DIAG						/* should comment when on board*/
#define CONFIG_BOOT_MOVINAND

#define CONFIG_COMMANDS				CFG_COMMANDS


#define disable_interrupts() __disable_irq()

typedef struct global_data {
	uint32_t	baudrate;
	uint32_t	ip_addr;
	uint8_t		enetaddr[6];
	char	*		gb_filename;
	uint8_t		env_valid;
	uint32_t	env_addr;
	uint32_t	env_size;
} gd_t;

extern gd_t * gd;
extern char console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/

int32_t		serial_tstc (void);
char	serial_getc (void);
char	serial_putc (char ch);
void	serial_puts (const char * s);
void	serial_setbrg (int32_t baudrate);
int32_t		console_init (void);

int32_t		tstc (void);
int32_t		ctrlc (void);
int32_t		disable_ctrlc (int32_t disable);
int32_t		had_ctrlc (void);
void	clear_ctrlc (void);

int32_t		readline (const char * const prompt);
int32_t		run_command (const char * cmd, int32_t flag);

void reset_cpu(void);
#endif
