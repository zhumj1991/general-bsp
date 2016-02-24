#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H

#include <stdint.h>

uint8_t *env_get_addr (int index);
void env_crc_update(void);
char *getenv (const char *name);
int32_t getenv_r (char *name, char *buf, unsigned len);
int8_t saveenv(void);
void gb_init(void);

#endif
