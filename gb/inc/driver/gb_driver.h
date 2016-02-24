#ifndef _GB_DRIVER_H_
#define _GB_DRIVER_H_
#include <ctype.h>
#include <stddef.h>
#include "gb_config.h"
#include "gb_list.h"
#include "gb_dev.h"
#include "rcc.h"
#include "uart.h"
#include "fsmc.h"
#include "interrupt.h"



#define __init_driver  __attribute__ ((unused, section (".driver")))

//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
//#define container_of(ptr, type, member) ({					\
//    const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
//    (type *)( (char *)__mptr - offsetof(type, member));})
#define container_of(ptr, type, member) 					\
			(type *)( (char *)ptr - offsetof(type, member))


struct attr {
	uint8_t init;
};

struct driver {
	char *name;
	uint32_t base;
	struct rcc *rcc;
	
	void *attr;
	
	sem_handle_t *read_sem;
	sem_handle_t *write_sem;

	int8_t (*gpio_init)(struct driver* driver);
	int8_t (*func_init)(struct driver * driver);
	int8_t (*read)(struct driver * driver, uint8_t * buf, uint8_t count);
	int8_t (*write)(struct driver * driver, uint8_t * buf, uint8_t count);
	int8_t (*ioctl)(struct driver * driver, uint8_t cmd, void * arg);
	int8_t (*close)(struct driver * driver);
};

struct driver_list {
	struct driver * driver;
	struct list_head list_head;
	struct dev_list * dev_list;
};

extern struct driver *(__init_driver_start);
extern struct driver *(__init_driver_end);

extern struct driver_list driver_list_head;

extern OS_MEM *driver_partition;
extern OS_MEM *dev_partition;


int8_t dev_mach(struct dev * dev);
struct driver * driver_find(const char *name);

#endif
