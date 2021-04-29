/*
 * touch.c
 *
 *  Created on: 2021. 4. 29.
 *      Author: baram
 */




#include "touch.h"
#include "touch/gt9147.h"

#include "cli.h"



#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif



static bool is_init = false;
static touch_driver_t touch;

bool touchInit(void)
{
  is_init = gt9147InitDriver(&touch);


  if (is_init)
  {
    if (touch.init() != true)
    {
      is_init = false;
    }
  }

#ifdef _USE_HW_CLI
  cliAdd("touch", cliCmd);
#endif
  return is_init;
}

uint8_t touchGetTouchedCount(void)
{
  if (is_init)
  {
    return touch.getTouchedCount();
  }
  else
  {
    return 0;
  }
}

bool touchGetTouchedData(uint8_t index, touch_data_t *p_data)
{
  if (is_init)
  {
    return touch.getTouchedData(index, p_data);
  }
  else
  {
    return false;
  }
}




#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = true;


  if (args->argc == 1  && args->isStr(0, "info") == true)
  {
    uint8_t touch_cnt;
    touch_data_t touch_data;

    while(cliKeepLoop())
    {
      touch_cnt = touchGetTouchedCount();
      if ( touch_cnt> 0)
      {
        cliPrintf("Touch : %d, ", touch_cnt);

        for (int i=0; i<touch_cnt; i++)
        {
          touchGetTouchedData(i, &touch_data);
          cliPrintf("[%d %d, x=%03d y=%03d w=%03d] ", touch_data.event, touch_data.id, touch_data.x, touch_data.y, touch_data.w);
        }
        cliPrintf("\n");
      }
      delay(10);
    }
  }
  else
  {
    ret = false;
  }


  if (ret == false)
  {
    cliPrintf( "touch info \n");
  }
}
#endif
