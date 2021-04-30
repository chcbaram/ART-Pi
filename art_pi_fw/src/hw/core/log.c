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

static osMutexId mutex_lock;


bool logInit(void)
{
  osMutexDef(mutex_lock);
  mutex_lock = osMutexCreate (osMutex(mutex_lock));

  return true;
}

void logPrintf(const char *fmt, ...)
{
  osMutexWait(mutex_lock, osWaitForever);

  va_list args;
  int len;

  va_start(args, fmt);
  len = vsnprintf(print_buf, 256, fmt, args);

  uartWrite(log_ch, (uint8_t *)print_buf, len);
  va_end(args);

  osMutexRelease(mutex_lock);
}



#endif
