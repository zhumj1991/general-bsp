#include <stdio.h>
#include <string.h>
#include "gb_driver.h"
#include "gb_dev.h"
#include "gb_fifo.h"
#include "uart.h"
#include "command.h"

/***************************************************************************
 * initial console
 **************************************************************************/
struct dev console_dev = {
	.name = "console",
	.driver_name = "uart_1",
};

int serial_tstc(void)
{
	struct driver * driver = console_dev.driver;
	struct uart_attr *attr = (struct uart_attr*)driver->attr;
	
	if(fifo_empty(attr->read_fifo))
		return 0;
	else
		return 1;
}

char serial_getc(void)
{
	char ch = 0;
	struct driver * driver = console_dev.driver;

//	while(!serial_tstc());
//	driver->read(driver, (unsigned char *)&ch, 1);
	while(0 == driver->read(driver, (unsigned char *)&ch, 1));
	
	return ch;
}

char serial_putc(char ch)
{
	struct driver * driver = console_dev.driver;
	driver->write(driver, (unsigned char *)&ch, 1);

	return ch;
}

void serial_puts(const char *s)
{
	struct driver * driver = console_dev.driver;
	driver->write(driver, (unsigned char *)s, strlen(s));
}

void serial_setbrg (int baudrate)
{
	struct driver * driver = console_dev.driver;
	
	driver->ioctl(driver, UART_BD_SET, (void *)&baudrate);

}

int console_init(void)
{
	if(dev_mach(&console_dev) == 0) {
		struct uart_attr *attr = (struct uart_attr *)console_dev.driver->attr;
		printf("[console_init]buadrate: %d\r\n", attr->baudrate);
		return 0;
	}

	return -1;
}
/***************************************************************************
 * check uartdata reday?
 **************************************************************************/
int tstc (void)
{
	return serial_tstc();
}

/***************************************************************************
 * test if ctrl-c was pressed
 **************************************************************************/
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;

int ctrlc (void)
{
	if (!ctrlc_disabled) {
		if(serial_tstc()) {
			switch (serial_getc()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				return 1;
			default:
				break;
			}
		}
	}
	return 0;
}

/***************************************************************************
 * pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 **************************************************************************/
int disable_ctrlc (int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc (void)
{
	ctrlc_was_pressed = 0;
}

/**************************************************************************/
char console_buffer[CFG_CBSIZE];				/* console I/O buffer	*/

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen);
static char erase_seq[] = "\b \b";			/* erase sequence	*/
static char   tab_seq[] = "        ";		/* used to expand TABs	*/

/***************************************************************************
 * delete char in console
 **************************************************************************/
static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			serial_puts (erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				serial_puts (tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				serial_putc (*s);
			}
		}
	} else {
		serial_puts (erase_seq);
		(*colp)--;
	}
	(*np)--;
	return (p);
}

/**************************************************************************
 * Prompt for input and read a line.
 * If  CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 * Return:	number of read characters
 *		-1 if break
 *		-2 if timed out
 **************************************************************************/
int readline (const char *const prompt)
{
	char   *p = console_buffer;
	int	n = 0;				/* buffer index		*/
	int	plen = 0;			/* prompt length	*/
	int	col;				/* output column cnt	*/
	char	c;
	
	/* print prompt */
	if (prompt) {
		plen = strlen (prompt);
		serial_puts (prompt);
	}
	col = plen;

	for (;;) {
		c = serial_getc();
			
		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			serial_puts ("\r\n");
			return (p - console_buffer);

		case '\0':				/* nul			*/
			continue;

		case 0x03:				/* ^C - break		*/
			console_buffer[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				serial_puts (erase_seq);
				--col;
			}
			p = console_buffer;
			n = 0;
			continue;

		case 0x17:				/* ^W - erase word 	*/
			p=delete_char(console_buffer, p, &col, &n, plen);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(console_buffer, p, &col, &n, plen);
			}
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(console_buffer, p, &col, &n, plen);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CFG_CBSIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
#ifdef CONFIG_AUTO_COMPLETE
					/* if auto completion triggered just continue */
					*p = '\0';
					if (cmd_auto_complete(prompt, console_buffer, &n, &col)) {
						p = console_buffer + n;	/* reset */
						continue;
					}
#endif
					serial_puts (tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					serial_putc (c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				serial_putc ('\a');
			}
		}
	}
}
