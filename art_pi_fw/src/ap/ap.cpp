/*
 * ap.cpp
 *
 *  Created on: Apr 19, 2021
 *      Author: baram
 */




#include "ap.h"
#include "thread/thread.h"
#include "TouchGFX/app/touchgfx.h"


void cliTest(cli_args_t *args);


void apInit(void)
{
  uartOpen(_DEF_UART2, 115200);

  cliAdd("test", cliTest);

  threadInit();

#ifdef _USE_HW_TOUCHGFX
  touchgfxInit();
#endif
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
    delay(1);
  }
}




void cliTest(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "hci") == true)
  {
    uint32_t pre_time = 0;
    uint8_t tx_buf[10];

    tx_buf[0] = 0x01;
    tx_buf[1] = 0x05;
    tx_buf[2] = 0x14;
    tx_buf[3] = 0;

    while(cliKeepLoop())
    {
      if (millis()-pre_time >= 3000)
      {
        pre_time = millis();

        uartWrite(_DEF_UART2, tx_buf, 4);
        logPrintf("tx \n");
      }

      if (uartAvailable(_DEF_UART2) > 0)
      {
        logPrintf("rx : 0x%X\n", uartRead(_DEF_UART2));
      }
    }

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "bt_spp") == true)
  {
    while(cliKeepLoop())
    {
      if (uartAvailable(_DEF_UART3) > 0)
      {
        uint8_t rx_data;

        rx_data = uartRead(_DEF_UART3);

        uartPrintf(_DEF_UART3, "RxData : 0x%X(%c)\n", rx_data, rx_data);
      }
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "usb") == true)
  {
    while(cliKeepLoop())
    {
      cliPrintf("USB Connect : %d\n", usbIsConnect());
      cliPrintf("USB Open    : %d\n", usbIsOpen());
      cliPrintf("\x1B[%dA", 2);
      delay(100);
    }
    cliPrintf("\x1B[%dB", 2);

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "usb_tx") == true)
  {
    uint32_t pre_time;
    uint32_t tx_cnt = 0;
    while(cliKeepLoop())
    {
      if (millis()-pre_time >= 1000)
      {
        pre_time = millis();
        logPrintf("tx : %d KB/s\n", tx_cnt/1024);
        tx_cnt = 0;
      }
      uartPrintf(_DEF_UART4, "123456789012345678901234567890\n");
      tx_cnt += 31;
    }
    cliPrintf("\x1B[%dB", 2);

    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("test hci\n");
    cliPrintf("test bt_spp\n");
    cliPrintf("test usb\n");
  }
}




