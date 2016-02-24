#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gb_driver.h"
#include "gb_dev.h"
#include "errno.h"
#include "gb_list.h"

struct driver_list driver_list_head;

int8_t driver_init(void)
{
	struct driver *driver_current;
	struct driver_list * driver_list;

	memset(&driver_list_head, 0, sizeof(struct driver_list));
	INIT_LIST_HEAD(&driver_list_head.list_head);
	
	for(driver_current = __init_driver_start;
			driver_current != __init_driver_end;
			driver_current++) {
		if(driver_current->gpio_init != 0) {
			if(driver_current->gpio_init(driver_current) != 0)
				continue;
		}
		
		if(driver_current->func_init != 0) {
			if(driver_current->func_init(driver_current) == 0) {
				((struct attr *)driver_current->attr)->init = 1;
				
				driver_list = (struct driver_list *)malloc(sizeof(struct driver_list));
				memset(driver_list, 0, sizeof(struct driver_list));
				driver_list->driver = driver_current;
				list_add_tail(&driver_list->list_head, &driver_list_head.list_head);	
				
				driver_list->dev_list = malloc(sizeof(struct dev_list));
				memset(driver_list->dev_list, 0, sizeof(struct dev_list));				
				INIT_LIST_HEAD(&driver_list->dev_list->list_head);						
			}
		}
	}
	
	return 0;
}
