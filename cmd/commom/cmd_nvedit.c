#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "environment.h"

static const unsigned long baudrate_table[] = {1200, 9600, 38400, 115200};
#define	N_BAUDRATES (sizeof(baudrate_table) / sizeof(baudrate_table[0]))

extern uchar (*env_get_char)(int);


static int envmatch (uchar *s1, int i2)
{
	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=') return(i2);
	
	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return(i2);

	return(-1);
}

/************************************************************************
 * Command interface: print one or all environment variables
 */
int do_printenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i, j, k, nxt;
	int rcode = 0;

	if (argc == 1) {		/* Print all env variables	*/
		for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
			for (nxt=i; env_get_char(nxt) != '\0'; ++nxt)
				;
			for (k=i; k<nxt; ++k)
				serial_putc(env_get_char(k));
			serial_puts  ("\r\n");

			if (ctrlc()) {
				serial_puts ("\r\n ** Abort\r\n");
				return 1;
			}
		}

		printf("\nEnvironment size: %d/%d bytes\r\n", i, ENV_SIZE);

		return 0;
	}

	for (i=1; i<argc; ++i) {	/* print single env variables	*/
		char *name = argv[i];

		k = -1;

		for (j=0; env_get_char(j) != '\0'; j=nxt+1) {

			for (nxt=j; env_get_char(nxt) != '\0'; ++nxt)
				;
			k = envmatch((uchar *)name, j);
			if (k < 0) {
				continue;
			}
			serial_puts (name);
			serial_putc ('=');
			while (k < nxt)
				serial_putc(env_get_char(k++));
			serial_puts ("\r\n");
			break;
		}
		if (k < 0) {
			printf ("## Error: \"%s\" not defined\r\n", name);
			rcode ++;
		}
	}
	return rcode;
}

/************************************************************************
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */

char *getenv (const char *name)
{
	int i, nxt;

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CFG_ENV_SIZE) {
				return (NULL);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;
		return ((char *)env_get_addr(val));
	}

	return (NULL);
}

int getenv_r (char *name, char *buf, unsigned len)
{
	int i, nxt;

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val, n;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CFG_ENV_SIZE) {
				return (-1);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;
		/* found; copy out */
		n = 0;
		while ((len > n++) && (*buf++ = env_get_char(val++)) != '\0')
			;
		if (len == n)
			*buf = '\0';
		return (n);
	}
	return (-1);
}

/************************************************************************
 * Set a new environment variable,
 * or replace or delete an existing one.
 *
 * This function will ONLY work with a in-RAM copy of the environment
 */

int _do_setenv (int flag, int argc, char *argv[])
{
	int   i, len, oldval;
	uchar *env, *nxt = NULL;
	char *name;

	uchar *env_data = (uchar *)env_get_addr(0);

	if (!env_data)	/* need copy in RAM */
		return 1;

	name = argv[1];

	if (strchr(name, '=')) {
		printf ("## Error: illegal character '=' in variable name \"%s\"\r\n", name);
		return 1;
	}

	/*
	 * search if variable with this name already exists
	 */
	oldval = -1;
	for (env=env_data; *env; env=nxt+1) {
		for (nxt=env; *nxt; ++nxt)
			;
		if ((oldval = envmatch((uchar *)name, env-env_data)) >= 0)
			break;
	}

	/*
	 * Delete any existing definition
	 */
	if (oldval >= 0) {
#ifndef CONFIG_ENV_OVERWRITE

		/*
		 * Ethernet Address and serial# can be set only once,
		 * ver is readonly.
		 */
		if ( (strcmp (name, "serial#") == 0) ||
		    ((strcmp (name, "ethaddr") == 0)
#if defined(CONFIG_OVERWRITE_ETHADDR_ONCE) && defined(CONFIG_ETHADDR)
		     && (strcmp ((char *)env_get_addr(oldval),MK_STR(CONFIG_ETHADDR)) != 0)
#endif	/* CONFIG_OVERWRITE_ETHADDR_ONCE && CONFIG_ETHADDR */
		    ) ) {
			printf ("Can't overwrite \"%s\"\r\n", name);
			return 1;
		}
#endif

		/*
		 * Switch to new baudrate if new baudrate is supported
		 */
		if (strcmp(argv[1],"baudrate") == 0) {
			int baudrate = strtoul(argv[2], NULL, 10);
			int i;
			for (i=0; i<N_BAUDRATES; ++i) {
				if (baudrate == baudrate_table[i])
					break;
			}
			if (i == N_BAUDRATES) {
				printf ("## Baudrate %d bps not supported\r\n",
					baudrate);
				return 1;
			}
			printf ("## Switch baudrate to %d bps and press ENTER ...\r\n",
				baudrate);
			gd->baudrate = baudrate;

			serial_setbrg (baudrate);
			mdelay(50);
			for (;;) {
				if (serial_getc() == '\r')
				      break;
			}
		}

		if (*++nxt == '\0') {
			if (env > env_data) {
				env--;
			} else {
				*env = '\0';
			}
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

#ifdef CONFIG_NET_MULTI
	if (strncmp(name, "eth", 3) == 0) {
		char *end;
		int   num = simple_strtoul(name+3, &end, 10);

		if (strcmp(end, "addr") == 0) {
			eth_set_enetaddr(num, argv[2]);
		}
	}
#endif


	/* Delete only ? */
	if ((argc < 3) || argv[2] == NULL) {
		env_crc_update ();
		return 0;
	}

	/*
	 * Append new definition at the end
	 */
	for (env=env_data; *env || *(env+1); ++env)
		;
	if (env > env_data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > ENV_SIZE - (env-env_data)
	 */
	len = strlen(name) + 2;
	/* add '=' for first arg, ' ' for all others */
	for (i=2; i<argc; ++i) {
		len += strlen(argv[i]) + 1;
	}
	if (len > (&env_data[ENV_SIZE]-env)) {
		printf ("## Error: environment overflow, \"%s\" deleted\r\n", name);
		return 1;
	}
	while ((*env = *name++) != '\0')
		env++;
	for (i=2; i<argc; ++i) {
		char *val = argv[i];

		*env = (i==2) ? '=' : ' ';
		while ((*++env = *val++) != '\0')
			;
	}

	/* end is marked with double '\0' */
	*++env = '\0';

	/* Update CRC */
	env_crc_update ();

	/*
	 * Some variables should be updated when the corresponding
	 * entry in the enviornment is changed
	 */

	if (strcmp(argv[1],"ethaddr") == 0) {
		char *s = argv[2];	/* always use only one arg */
		char *e;
		for (i=0; i<6; ++i) {
			gd->enetaddr[i] = s ? strtoul(s, &e, 16) : 0;
			if (s) s = (*e) ? e+1 : e;
		}
#ifdef CONFIG_NET_MULTI
		eth_set_enetaddr(0, argv[2]);
#endif
		return 0;
	}

	if (strcmp(argv[1],"ipaddr") == 0) {
		char *s = argv[2];	/* always use only one arg */
		char *e;
		unsigned long addr;
		gd->ip_addr = 0;
		for (addr=0, i=0; i<4; ++i) {
			ulong val = s ? strtoul(s, &e, 10) : 0;
			addr <<= 8;
			addr  |= (val & 0xFF);
			if (s) s = (*e) ? e+1 : e;
		}
		gd->ip_addr = addr;//htonl(addr);
		return 0;
	}

#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if (strcmp(argv[1],"bootfile") == 0) {
		copy_filename (BootFile, argv[2], sizeof(BootFile));
		return 0;
	}
#endif	/* CFG_CMD_NET */

	return 0;
}
		
void setenv (char *varname, char *varvalue)
{
	char *argv[4] = { "setenv", varname, varvalue, NULL };
	_do_setenv (0, 3, argv);
}

int do_setenv ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
		return 1;
	}

	return _do_setenv (flag, argc, argv);
}

#if defined(CFG_ENV_IN_SD) || (CFG_CMD_ENV | CFG_SPI_FLASH)
int do_saveenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf ("Saving Environment...\r\n");

	return (saveenv() ? 1 : 0);
}
#endif

/*********************************************************************/
DIAG_CMD(
	printenv, CFG_MAXARGS, 1,	do_printenv,
	"printenv- print environment variables\r\n",
	"\r\n    - print values of all environment variables\r\n"
	"printenv name ...\r\n"
	"    - print value of environment variable 'name'\r\n"
);

DIAG_CMD(
	setenv, CFG_MAXARGS, 0,	do_setenv,
	"setenv  - set environment variables\r\n",
	"name value ...\r\n"
	"    - set environment variable 'name' to 'value ...'\r\n"
	"setenv name\r\n"
	"    - delete environment variable 'name'\r\n"
);


#if defined(CFG_ENV_IN_SD) || (CFG_CMD_ENV | CFG_SPI_FLASH)
DIAG_CMD(
	saveenv, 1, 0,	do_saveenv,
	"saveenv - save environment variables to persistent storage\r\n",
	NULL
);
#endif
