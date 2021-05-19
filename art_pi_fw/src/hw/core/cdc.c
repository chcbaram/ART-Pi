/*
 * cdc.c
 *
 *  Created on: 2021. 5. 20.
 *      Author: baram
 */




#include "cdc.h"


#ifdef _USE_HW_CDC


extern bool     cdcIfInit(void);
extern uint32_t cdcIfAvailable(void);
extern uint8_t  cdcIfRead(void);


static bool is_init = false;

bool cdcInit(void)
{
  bool ret = true;


  ret = cdcIfInit();

  is_init = ret;

  return ret;
}

bool cdcIsInit(void)
{
  return is_init;
}

uint32_t cdcAvailable(void)
{
  return cdcIfAvailable();
}

uint8_t cdcRead(void)
{
  return cdcIfRead();
}

uint32_t cdcWrite(uint8_t *p_data, uint32_t length)
{
  return 0;
}

uint32_t cdcGetBaud(void)
{
  return 0;
}

#endif
