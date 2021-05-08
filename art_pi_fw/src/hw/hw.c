/*
 * hw.c
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */


#include "hw.h"





void hwInit(void)
{
  cliInit();
  ledInit();
  uartInit();
  uartOpen(_DEF_UART1, 57600);

  logInit();
  logPrintf("\n\n");
  logPrintf("[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Clock\t\t: %d Mhz\r\n", (int)HAL_RCC_GetSysClockFreq()/1000000);

  gpioInit();
  i2cInit();
  buttonInit();
  touchInit();

  sdramInit();
  //qspiInit();
  spiFlashInit();
  flashInit();


  sdInit();
  fatfsInit();
  fsInit();

  lcdInit();
  btSppInit();

  logPrintf("\n");
}
