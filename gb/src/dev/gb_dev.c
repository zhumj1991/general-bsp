#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "gb_driver.h"
#include "gb_dev.h"
#include "gb_list.h"

void dev_add(struct dev *dev, struct dev_list * dev_list)
{
	OS_ERR err;
	struct dev_list *dev_cur;
	//dev_cur = malloc(sizeof(struct dev_list));
	dev_cur = (struct dev_list *)OSMemGet(driver_partition, &err);
	if(err == OS_ERR_NONE) {
		dev_cur->dev = dev;
		list_add_tail(&dev_cur->list_head, &dev_list->list_head);
	}
}


int dev_mach(struct dev * dev)
{
	struct list_head * pos;
	struct list_head * head = &driver_list_head.list_head;
	
	list_for_each(pos, head) {
		struct driver_list * driver_list = list_entry(pos, struct driver_list, list_head);
		
		if(!strcmp(dev->driver_name, driver_list->driver->name)) {
			struct attr * attr = (struct attr *)(driver_list->driver->attr);
			if(attr->init == 1) {
				dev->driver = driver_list->driver;
				dev_add(dev, driver_list->dev_list);
			} else
				return -1;

			return 0;
		}
	}
	
	return -1;
}
