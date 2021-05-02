/*
 * info.cpp
 *
 *  Created on: 2021. 5. 1.
 *      Author: baram
 */




#include "info.h"


static const char *thread_name = "info        ";
static thread_t *thread = NULL;


static void infoThread(void const *argument);
static bool infoEvent(Event_t event);
static void cliInfo(cli_args_t *args);





bool infoThreadInit(thread_t *p_thread)
{
  bool ret = false;

  thread = p_thread;

  thread->name = thread_name;
  thread->onEvent = infoEvent;

  cliAdd("info", cliInfo);

  osThreadDef(infoThread, infoThread, _HW_DEF_RTOS_THREAD_PRI_INFO, 0, _HW_DEF_RTOS_THREAD_MEM_INFO);
  if (osThreadCreate(osThread(infoThread), NULL) != NULL)
  {
    ret = true;
    logPrintf("infoThread \t\t: OK\r\n");
  }
  else
  {
    logPrintf("infoThread \t\t: Fail\r\n");
  }

  p_thread->is_init = ret;

  return ret;
}

void infoThread(void const *argument)
{
  (void)argument;
  static uint32_t pre_heartbeat[THREAD_ID_MAX];

  thread->is_start = true;


  while(1)
  {
    for (int i=0; i<THREAD_ID_MAX; i++)
    {
      pre_heartbeat[i] = thread->list[i].hearbeat;
    }

    delay(1000);
    thread->hearbeat++;

    for (int i=0; i<THREAD_ID_MAX; i++)
    {
      thread->list[i].freq = thread->list[i].hearbeat - pre_heartbeat[i];
    }
  }
}

bool infoEvent(Event_t event)
{
  bool ret = true;

  switch(event)
  {
    case EVENT_TOUCH_PRESSED:
      logPrintf("EVENT_TOUCH_PRESSED : %s,%d\n", __FILE__, __LINE__);
      break;

    case EVENT_TOUCH_RELEASED:
      logPrintf("EVENT_TOUCH_RELEASED : %s,%d\n", __FILE__, __LINE__);
      break;

    default:
      ret = false;
      break;
  }

  return ret;
}



void cliInfo(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "cpu"))
  {
    while(cliKeepLoop())
    {
      cliPrintf("cpu usage : %d %%\r", osGetCPUUsage());
      delay(100);
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "thread"))
  {
    while(cliKeepLoop())
    {
      cliPrintf("cpu usage : %d %%\n", osGetCPUUsage());

      for (int i=0; i<THREAD_ID_MAX; i++)
      {
        cliPrintf("%s \t: %04X, %04dHz\n",
                  thread->list[i].name,
                  (uint16_t)thread->list[i].hearbeat,
                  thread->list[i].freq
                  );
      }
      cliPrintf("\x1B[%dA", THREAD_ID_MAX+1);

      delay(100);
    }
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("info cpu\n");
    cliPrintf("info thread\n");
  }
}
