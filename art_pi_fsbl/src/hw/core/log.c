/*
 * log.c
 *
 *  Created on: 2021. 4. 30.
 *      Author: baram
 */




#include "log.h"
#include "uart.h"


#ifdef _USE_HW_LOG


static uint8_t log_ch = LOG_CH;
static char print_buf[256];




bool logInit(void)
{
  return true;
}

void logPrintf(const char *fmt, ...)
{
  va_list args;
  int len;

  va_start(args, fmt);
  len = vsnprintf(print_buf, 256, fmt, args);

  uartWrite(log_ch, (uint8_t *)print_buf, len);
  va_end(args);
}



#endif
