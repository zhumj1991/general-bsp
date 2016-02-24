#include <stdio.h>
#include <string.h>
#include "command.h"
#include "gb_driver.h"


#define DRIVER_TYPE			0
#define DEVICE_TYPE			1

const static char c_last[2][5] = {{'|', '-', '-', 0x00}, {0xA9, 0xB8, '-', '-', 0x00}};

static int tree(unsigned char type)
{
	struct list_head * driver_pos;
	struct list_head * driver_head = &driver_list_head.list_head;
	
	if(list_empty(driver_head) || (type != DRIVER_TYPE && type != DEVICE_TYPE))
		return -1;
	
	list_for_each(driver_pos, driver_head)
	{
		struct driver_list * driver_list = list_entry(driver_pos, struct driver_list, list_head);
		
		unsigned char driver_last = list_is_last(driver_pos, driver_head);
		if (type == DRIVER_TYPE)
			printf("  %s%s\r\n", c_last[driver_last], driver_list->driver->name);
		
		struct list_head * dev_pos;	
		struct list_head * dev_head = &driver_list->dev_list->list_head;
		unsigned char dev_last = 0;
	
		if(list_empty(dev_head))
			continue;
		
		list_for_each(dev_pos, dev_head)
		{
			struct dev_list *dev_cur = list_entry(dev_pos, struct dev_list, list_head);
			dev_last = list_is_last(dev_pos, dev_head);
			
			if (type == DRIVER_TYPE) {
				printf("  %c", driver_last?' ':'|');
				printf("  %s%s\r\n", c_last[dev_last], dev_cur->dev->name);
			} else
				printf("  %s%s\r\n", c_last[driver_last], dev_cur->dev->name);
		}	
	}
	
	return 0;
}

static void driver_attr(char *name)
{
	struct driver * driver;
	
	if((driver = driver_find(name)) != 0) {
		driver->ioctl(driver, 0, 0);
	} else
	printf(" Error: %s not found\r\n", name);
}

int do_driver (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc == 1)
		tree (DRIVER_TYPE);
	else if (argc == 2)
		driver_attr (argv[1]);
	else
		printf ("Usage:\r\n%s", cmdtp->usage);
				
	printf("\r\n");
	return 0;
}

DIAG_CMD(
	driver, 2, 1, do_driver,
 	"driver  - print driver tree\r\n",
	"\r\n    - print tree of all drivers\r\n"
	"driver name ...\r\n"
	"    - print attribution of driver 'name'\r\n"
);

int do_device (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc == 1) {
		tree(DEVICE_TYPE);
	}
	else
		printf ("Usage:\r\n%s\r\n", cmdtp->usage);
	
	return 0;
}

DIAG_CMD(
	device, 1, 1, do_device,
 	"device  - print device tree\r\n",
	NULL
);

