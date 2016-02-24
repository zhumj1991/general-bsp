#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "task.h"
#include "command.h"
#include "environment.h"

#if (TASK_DIAG_ENABLE)

static int abortboot(int bootdelay)
{
	int abort = 0;
	OS_ERR	err;
	
	if (bootdelay >= 0) {
		if (tstc()) {	/* we got a key press	*/
			(void) serial_getc();  /* consume input	*/
			serial_puts ("\b\b\b 0");
			abort = 1; 	/* don't auto boot	*/
		}
	}

	while ((bootdelay > 0) && (!abort)) {
		int i;

		--bootdelay;
		/* delay 100 * 10ms */
		for (i=0; !abort && i<100; ++i) {
			if (tstc()) {	/* we got a key press	*/
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
#ifdef CONFIG_MENUKEY
				menukey = serial_getc();
#else
				(void) serial_getc();  /* consume input	*/
#endif
				break;
			}
			OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &err);
		}
		printf ("\b\b\b%2d ", bootdelay);
	}
	serial_puts ("\n\r\n");

	return abort;
}


void task_diag (void *p_arg)
{
	char *s;
	
#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	int bootdelay;
	s = getenv ("bootdelay");
	bootdelay = s ? (int)strtol(s, NULL, 10) : CONFIG_BOOTDELAY;
#endif
	
#if defined(CONFIG_DIAG)
	printf ("\r\n#Hit any key to start diagnostic system: %2d ", bootdelay);
	if (bootdelay >= 0 && !abortboot (bootdelay)) 
#endif
	{
		int prev = disable_ctrlc(1);
		serial_puts ("#Stop diagnostic system, you cann't type any word now.\r\n");
		while(1) {
			if(0)
				break;
			mdelay(1000);
		}
		disable_ctrlc(prev);
	}	
	
	static char lastcommand[CFG_CBSIZE] = {0, };
	int len;
	int rc = 1;
	int flag;
	
#ifdef CONFIG_PASSWORD  
	char pwd[16]; 
	char	c;  
	int index;  
	static unsigned char bPwd = 1;
	serial_puts ("#Please input password:\r\n");	
	while (bPwd){		
			serial_puts ("#Password: ");
			index = 0;  
			while ((c = serial_getc()) != '\r'){				
					if (c == 8) /* Backspace */  
					{  
							if (index > 0){  
									printf ("\b \b");  
									index--;  
							}  
							continue;  
					} else if (c == 3){ /* Ctrl + c */  
              reset_cpu();
						break;
          }
					if(index < 16) {
						serial_putc('*');  
						pwd[index] = c;  
						index++;
					}
			}  
			pwd[index] = '\0';  
			serial_puts ("\r\n");  
  
			s = getenv ("password");  
			if (!s){  
					s = "geenovo";  
			}  
			if (!strcmp (pwd, s)) {
					bPwd = 0;  
			}
//			else
//				serial_puts ("Error password, Try again!\r\n");
	}  
#endif
	serial_puts ("#Welcom to diagnostic system!\r\n");
	while(1) {
		len = readline (CFG_PROMPT);

		flag = 0;	/* assume no special flags for now */
		if (len > 0)
			strcpy (lastcommand, console_buffer);
		else if (len == 0)
			flag |= CMD_FLAG_REPEAT;

		if (len == -1)
			serial_puts ("<INTERRUPT>\r\n");
		else
			rc = run_command (lastcommand, flag);

		if (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}
#endif
