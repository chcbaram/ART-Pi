/*
 * main.cpp
 *
 *  Created on: Apr 19, 2021
 *      Author: baram
 */




#include "main.h"





static void mainThread(void const *argument);


int main(void)
{
  bspInit();


  osThreadDef(mainThread, mainThread, _HW_DEF_RTOS_THREAD_PRI_MAIN, 0, _HW_DEF_RTOS_THREAD_MEM_MAIN);
  if (osThreadCreate(osThread(mainThread), NULL) == NULL)
  {
    logPrintf("threadMain \t\t: Fail\r\n");
    while(1);
  }

  osKernelStart();
  return 0;
}

void mainThread(void const *argument)
{
  UNUSED(argument);

  hwInit();
  apInit();
  apMain();
}

