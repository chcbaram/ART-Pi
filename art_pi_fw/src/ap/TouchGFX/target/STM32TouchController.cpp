/**
  ******************************************************************************
  * File Name          : STM32TouchController.cpp
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* USER CODE BEGIN STM32TouchController */

#include <STM32TouchController.hpp>
#include "touch.h"

void STM32TouchController::init()
{
    /**
     * Initialize touch controller and driver
     *
     */
#if 0
    TS_Init_t hTS;
    hTS.Orientation = TS_SWAP_XY;
    hTS.Accuracy = 0;
    hTS.Width = 480;
    hTS.Height = 272;
    BSP_TS_Init(0, &hTS);
#endif
}

bool STM32TouchController::sampleTouch(int32_t& x, int32_t& y)
{
    /**
     * By default sampleTouch returns false,
     * return true if a touch has been detected, otherwise false.
     *
     * Coordinates are passed to the caller by reference by x and y.
     *
     * This function is called by the TouchGFX framework.
     * By default sampleTouch is called every tick, this can be adjusted by HAL::setTouchSampleRate(int8_t);
     *
     */
#if 0
    TS_State_t  TS_State = { 0 };
    BSP_TS_GetState(0, &TS_State);
    if (TS_State.TouchDetected)
    {
        x = TS_State.TouchX;
        y = TS_State.TouchY;
        return true;
    }
#endif


    uint8_t touch_cnt;

    touch_cnt = touchGetTouchedCount();
    if (touch_cnt > 0)
    {
      touch_data_t data;

      touchGetTouchedData(0, &data);

      x = data.x;
      y = data.y;

      return true;
    }

    return false;
}

/* USER CODE END STM32TouchController */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
