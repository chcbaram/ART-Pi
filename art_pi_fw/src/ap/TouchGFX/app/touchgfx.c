/*
 * touchgfx.c
 *
 *  Created on: 2021. 5. 27.
 *      Author: baram
 */




#include "touchgfx.h"





void touchgfx_init(void);
void touchgfx_taskEntry(void);

static void TouchGFX_Task(void const *argument);


bool touchgfxInit(void)
{
  bool ret = false;


  touchgfx_init();

  osThreadDef(TouchGFX_Task, TouchGFX_Task, _HW_DEF_RTOS_THREAD_PRI_TOUCHGFX, 0, _HW_DEF_RTOS_THREAD_MEM_TOUCHGFX);
  if (osThreadCreate(osThread(TouchGFX_Task), NULL) != NULL)
  {
    ret = true;
    logPrintf("TouchGFX_Task \t\t: OK\r\n");
  }
  else
  {
    logPrintf("TouchGFX_Task \t\t: Fail\r\n");
  }

  return ret;
}

void TouchGFX_Task(void const *argument)
{
  // Calling farward to touchgfx_init in C++ domain
  touchgfx_taskEntry();
}
