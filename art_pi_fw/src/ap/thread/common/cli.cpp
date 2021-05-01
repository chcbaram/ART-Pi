/*
 * cli.cpp
 *
 *  Created on: 2021. 5. 1.
 *      Author: baram
 */



#include "cli.h"


static const char *thread_name = "cli         ";
static thread_t *thread = NULL;


static void cliThread(void const *argument);;





bool cliThreadInit(thread_t *p_thread)
{
  bool ret = false;

  thread = p_thread;

  thread->name = thread_name;

  osThreadDef(cliThread, cliThread, _HW_DEF_RTOS_THREAD_PRI_CLI, 0, _HW_DEF_RTOS_THREAD_MEM_CLI);
  if (osThreadCreate(osThread(cliThread), NULL) != NULL)
  {
    ret = true;
    logPrintf("cliThread \t\t: OK\r\n");
  }
  else
  {
    logPrintf("cliThread \t\t: Fail\r\n");
  }

  p_thread->is_init = ret;

  return ret;
}

void cliThread(void const *argument)
{
  (void)argument;

  cliOpen(_DEF_UART1, 57600);

  while(1)
  {
    cliMain();
    delay(5);
    thread->hearbeat++;
  }
}


