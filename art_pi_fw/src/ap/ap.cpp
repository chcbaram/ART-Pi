/*
 * ap.cpp
 *
 *  Created on: Apr 19, 2021
 *      Author: baram
 */




#include "ap.h"
#include "thread/thread.h"




void apInit(void)
{
  threadInit();
}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }

    sd_state_t sd_state;

    sd_state = sdUpdate();
    if (sd_state == SDCARD_CONNECTED)
    {
      threadNotify(EVENT_SDCARD_CONNECTED);
    }
    if (sd_state == SDCARD_DISCONNECTED)
    {
      threadNotify(EVENT_SDCARD_DISCONNECTED);
    }

    delay(10);
  }
}







