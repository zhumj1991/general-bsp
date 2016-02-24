#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "task.h"
#include "command.h"

#include "lwIP/tcp.h"
#include "lwIP/udp.h"
#include "lwIP/dhcp.h"
#include "lwip/memp.h"
#include "lwIP/tcpip.h"
#include "netif/etharp.h"
#include "arch/sys_arch.h"



#if TASK_ETH_ENABLE


void task_eth (void *p_arg)
{		
	while(1)
	{
		mdelay(1000);
	}
}
	
#endif
