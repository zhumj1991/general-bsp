/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
/*  Porting by Michael Vysotsky <michaelvy@hotmail.com> August 2011   */

#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"

#include "arch/sys_arch.h"

/*----------------------------------------------------------------------------*/
/*                      DEFINITIONS                                           */
/*----------------------------------------------------------------------------*/
#include "os_cfg_app.h"
#define LWIP_ARCH_TICK_PER_MS       1000/OS_CFG_TICK_RATE_HZ



/*----------------------------------------------------------------------------*/
/*                      VARIABLES                                             */
/*----------------------------------------------------------------------------*/

static OS_MEM StackMem;
const void * const pvNullPointer = (mem_ptr_t*)0xffffffff;
 
 
#pragma pack(4)
CPU_STK       LwIP_Task_Stk[LWIP_TASK_MAX * LWIP_STK_SIZE];
#pragma pack(4)
CPU_INT08U    LwIP_task_priopity_stask[LWIP_TASK_MAX];
OS_TCB        LwIP_task_TCB[LWIP_TASK_MAX]; 


err_t sys_mbox_new (sys_mbox_t *mbox, int size)
{
  OS_ERR       err;
      
  OSQCreate(mbox,"LWIP quiue", size, &err); 
  LWIP_ASSERT( "OSQCreate ", err == OS_ERR_NONE );
  
  if( err == OS_ERR_NONE)
  { 
    return 0; 
  }
  return -1;
}

void sys_mbox_free (sys_mbox_t * mbox)
{
    OS_ERR     err;
    LWIP_ASSERT( "sys_mbox_free ", mbox != SYS_MBOX_NULL );      
        
    OSQFlush(mbox,& err);
    
    OSQDel(mbox, OS_OPT_DEL_ALWAYS, &err);
    LWIP_ASSERT( "OSQDel ", err == OS_ERR_NONE );
}


void sys_mbox_post (sys_mbox_t *mbox, void *msg)
{
  OS_ERR     err;
  CPU_INT08U  i=0; 
  if( msg == NULL ) 
	 msg = (void*)&pvNullPointer;
  /* try 10 times */
  while(i<10)
  {
    OSQPost(mbox, msg,0,OS_OPT_POST_ALL,&err);
    if(err == OS_ERR_NONE)
      break;
    i++;
    OSTimeDly(5,OS_OPT_TIME_DLY,&err);
  }
  LWIP_ASSERT( "sys_mbox_post error!\n", i !=10 );  
}


err_t sys_mbox_trypost (sys_mbox_t *mbox, void *msg)
{
  OS_ERR     err;
  if(msg == NULL )
	  msg = (void*)&pvNullPointer;  
  OSQPost(mbox, msg,0,OS_OPT_POST_ALL,&err);    
  if(err != OS_ERR_NONE)
  {
    return ERR_MEM;
  }
  return ERR_OK;
}

u32_t sys_arch_mbox_fetch (sys_mbox_t *mbox, void **msg, u32_t timeout)
{ 
  OS_ERR	err;
  OS_MSG_SIZE   msg_size;
  CPU_TS        ucos_timeout;  
  CPU_TS        in_timeout = timeout/LWIP_ARCH_TICK_PER_MS;
  if(timeout && in_timeout == 0)
    in_timeout = 1;
  *msg  = OSQPend (mbox,in_timeout,OS_OPT_PEND_BLOCKING,&msg_size, &ucos_timeout,&err);
  if ( err == OS_ERR_TIMEOUT ) 
      ucos_timeout = SYS_ARCH_TIMEOUT;  
  return ucos_timeout; 
}

u32_t sys_arch_mbox_tryfetch (sys_mbox_t *mbox, void **msg)
{
	return sys_arch_mbox_fetch(mbox,msg,1);
}

int sys_mbox_valid (sys_mbox_t *mbox)
{
  if(mbox->NamePtr)  
    return (strcmp(mbox->NamePtr,"?Q"))? 1:0;
  else
    return 0;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  if(sys_mbox_valid(mbox))
    sys_mbox_free(mbox);
}

err_t sys_sem_new (sys_sem_t * sem, u8_t count)
{  
  OS_ERR	err;
  OSSemCreate (sem, "LWIP Sem", count, &err);
  if(err != OS_ERR_NONE )
	{
    LWIP_ASSERT("OSSemCreate ", err == OS_ERR_NONE);
    return -1;    
	}
  return 0;
}

u32_t sys_arch_sem_wait (sys_sem_t *sem, u32_t timeout)
{ 
  OS_ERR	err;
  CPU_TS        ucos_timeout;
  CPU_TS        in_timeout = timeout/LWIP_ARCH_TICK_PER_MS;
  if(timeout && in_timeout == 0)
    in_timeout = 1;  
  OSSemPend (sem, in_timeout, OS_OPT_PEND_BLOCKING, &ucos_timeout, &err);
	
	/*  only when timeout! */
  if(err == OS_ERR_TIMEOUT)
      ucos_timeout = SYS_ARCH_TIMEOUT;	
  return ucos_timeout;
}

void sys_sem_signal(sys_sem_t *sem)
{
  OS_ERR	err;  
  OSSemPost(sem,OS_OPT_POST_ALL,&err);
  LWIP_ASSERT("OSSemPost ",err == OS_ERR_NONE );  
}

void sys_sem_free(sys_sem_t *sem)
{
    OS_ERR     err;
    OSSemDel(sem, OS_OPT_DEL_ALWAYS, &err );
    LWIP_ASSERT( "OSSemDel ", err == OS_ERR_NONE );
}

int sys_sem_valid(sys_sem_t *sem)
{
  if(sem->NamePtr)
    return (strcmp(sem->NamePtr,"?SEM"))? 1:0;
  else
    return 0;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
  if(sys_sem_valid(sem))
    sys_sem_free(sem);
}

void sys_init(void)
{
  OS_ERR err;
  memset(LwIP_task_priopity_stask,0,sizeof(LwIP_task_priopity_stask));
  /* init mem used by sys_mbox_t, use ucosII functions */
  OSMemCreate(&StackMem, "LWIP TASK STK", (void*)LwIP_Task_Stk, LWIP_TASK_MAX,LWIP_STK_SIZE*sizeof(CPU_STK),&err);
  LWIP_ASSERT( "sys_init: failed OSMemCreate STK", err == OS_ERR_NONE );
}
 
extern CPU_STK * TCPIP_THREAD_TASK_STK;

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    int i;
    OS_ERR      err;
	int tsk_prio;
	CPU_STK * task_stk;
//	=(CPU_STK * )0;
	CPU_INT08U  ubPrio = LWIP_TASK_START_PRIO;

    arg = arg;
     
    if(prio)
	{
      ubPrio +=(prio-1);
      for(i=0; i<LWIP_TASK_MAX; ++i)
        if(LwIP_task_priopity_stask[i] == ubPrio)
          break;
      if(i == LWIP_TASK_MAX)
		{
        for(i=0; i<LWIP_TASK_MAX; ++i)
          if(LwIP_task_priopity_stask[i]==0)
		  {
            LwIP_task_priopity_stask[i] = ubPrio;
            break;
          }
        if(i == LWIP_TASK_MAX)
		{
          LWIP_ASSERT( "sys_thread_new: there is no space for priority", 0 );
          return (-1);
        }        
      }else
        prio = 0;
    }
  /* Search for a suitable priority */     
    if(!prio)
	{
      ubPrio = LWIP_TASK_START_PRIO;
      while(ubPrio < (LWIP_TASK_START_PRIO+LWIP_TASK_MAX))
	  { 
        for(i=0; i<LWIP_TASK_MAX; ++i)
          if(LwIP_task_priopity_stask[i] == ubPrio)
		  {
            ++ubPrio;
            break;
          }
        if(i == LWIP_TASK_MAX)
          break;
      }
      if(ubPrio < (LWIP_TASK_START_PRIO+LWIP_TASK_MAX))
        for(i=0; i<LWIP_TASK_MAX; ++i)
          if(LwIP_task_priopity_stask[i]==0)
		  {
            LwIP_task_priopity_stask[i] = ubPrio;
            break;
          }
      if(ubPrio >= (LWIP_TASK_START_PRIO+LWIP_TASK_MAX) || i == LWIP_TASK_MAX)
	  {
        LWIP_ASSERT( "sys_thread_new: there is no free priority", 0 );
        return (-1);
      }
    }
    if(stacksize > LWIP_STK_SIZE || !stacksize)   
        stacksize = LWIP_STK_SIZE;
  /* get Stack from pool */
    task_stk = OSMemGet( &StackMem, &err );
    if(err != OS_ERR_NONE)
	{
      LWIP_ASSERT( "sys_thread_new: impossible to get a stack", 0 );
      return (-1);
    } 
    tsk_prio = ubPrio-LWIP_TASK_START_PRIO;
    OSTaskCreate(&LwIP_task_TCB[tsk_prio],
                 (CPU_CHAR  *)name,
                 (OS_TASK_PTR)thread, 
                 (void      *)0,
                 (OS_PRIO    )ubPrio,
                 (CPU_STK   *)&task_stk[0],
                 (CPU_STK_SIZE)stacksize/10,
                 (CPU_STK_SIZE)stacksize,
                 (OS_MSG_QTY )0,
                 (OS_TICK    )0,
                 (void      *)0,
                 (OS_OPT     )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR    *)&err);  
						
    return ubPrio;
}

//void sys_msleep(u32_t ms)
//{
//  OS_ERR      err;  
//  OSTimeDly(ms, OS_OPT_TIME_DLY, &err);  
//}

