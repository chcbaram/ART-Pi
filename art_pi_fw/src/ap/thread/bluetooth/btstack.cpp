/*
 * btstack.cpp
 *
 *  Created on: 2021. 5. 10.
 *      Author: baram
 */




#include "btstack.h"


static const char *thread_name = "btstack      ";
static thread_t *thread = NULL;


static void btstackThread(void const *argument);;





bool btstackThreadInit(thread_t *p_thread)
{
  bool ret = false;

  thread = p_thread;

  thread->name = thread_name;

  osThreadDef(btstackThread, btstackThread, _HW_DEF_RTOS_THREAD_PRI_BTSTACK, 0, _HW_DEF_RTOS_THREAD_MEM_BTSTACK);
  if (osThreadCreate(osThread(btstackThread), NULL) != NULL)
  {
    ret = true;
    logPrintf("btstackThread \t\t: OK\r\n");
  }
  else
  {
    logPrintf("btstackThread \t\t: Fail\r\n");
  }

  p_thread->is_init = ret;

  return ret;
}

void btstackThread(void const *argument)
{
  (void)argument;
  bool bt_open;
  bool bt_open_pre;

  bt_open = btSppIsOpen();
  bt_open_pre = !bt_open;

  while(1)
  {
    btSppExcute();
    delay(1);

    bt_open = btSppIsOpen();
    if (bt_open != bt_open_pre)
    {
      if (bt_open == true)
      {
        thread->notify(EVENT_BT_OPEN);
      }
      else
      {
        thread->notify(EVENT_BT_CLOSE);
      }
    }
    bt_open_pre = bt_open;
    thread->hearbeat++;
  }
}


