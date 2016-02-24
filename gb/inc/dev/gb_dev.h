#ifndef _GB_DEV_H_
#define _GB_DEV_H_

#include <stdint.h>
#include "gb_list.h"

struct dev {
	char * name;
	char * driver_name;
	struct driver * driver;
	void * func;
};

struct dev_list {
	struct dev *dev;
	struct list_head list_head;
};

int8_t dev_mach(struct dev * dev);


#endif
