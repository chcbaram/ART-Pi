/*
 * ap.cpp
 *
 *  Created on: Apr 19, 2021
 *      Author: baram
 */




#include "ap.h"





void apInit(void)
{
  void (**jump_func)(void) = (void (**)(void))(0x90000000 + 4);
  uint32_t jump_addr;


  cliOpen(_DEF_UART1, 57600);

  jump_addr = (uint32_t)(*jump_func);

  if (jump_addr > 0x90000000 && jump_addr < (0x90000000+16*1024*1024))
  {
    logPrintf("Jump To 0x%X\n", jump_addr);
    delay(50);
    bspDeInit();
    (*jump_func)();
  }
  else
  {
    logPrintf("Invalid Jump Address 0x%X\n", jump_addr);
  }
}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 100)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }

    cliMain();

    sdUpdate();
  }
}







