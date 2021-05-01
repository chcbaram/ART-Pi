/*
 * display.cpp
 *
 *  Created on: 2021. 5. 1.
 *      Author: baram
 */




#include "display.h"


static const char *thread_name = "display     ";
static thread_t *thread = NULL;


static void displayThread(void const *argument);





bool displayThreadInit(thread_t *p_thread)
{
  bool ret = false;

  thread = p_thread;

  thread->name = thread_name;


  osThreadDef(displayThread, displayThread, _HW_DEF_RTOS_THREAD_PRI_DISPLAY, 0, _HW_DEF_RTOS_THREAD_MEM_DISPLAY);
  if (osThreadCreate(osThread(displayThread), NULL) != NULL)
  {
    ret = true;
    logPrintf("displayThread \t\t: OK\r\n");
  }
  else
  {
    logPrintf("displayThread \t\t: Fail\r\n");
  }

  p_thread->is_init = ret;

  return ret;
}

void displayThread(void const *argument)
{
  (void)argument;
  uint32_t pre_time;
  uint16_t x = 0;
  uint16_t y = 0;
  uint32_t show_data[3] = {0, };
  uint32_t pre_time_draw;
  uint32_t draw_time = 0;
  float char_size = 1.0;

  thread->is_start = true;



  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();

      show_data[0] = lcdGetFpsTime();
      show_data[1] = lcdGetFps();
      show_data[2] = draw_time;
    }


    if (lcdDrawAvailable())
    {
      pre_time_draw = millis();
      lcdClearBuffer(black);


      lcdPrintfResize(0, 16*0, white, 2, "LCD : %d ms, %d fps, %d ms", show_data[0], show_data[1], show_data[2]);


      lcdPrintfResize(0, 16*4, white, 1.5, "BTN : ");
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        lcdPrintfResize(i*8+(8*6)*1.5, 16*4, white, 1.5, "%d", buttonGetPressed(i));
      }

      lcdPrintfResize(0, 16*6, white, 1.5, "PIN : ");
      for (int i=0; i<GPIO_MAX_CH; i++)
      {
        lcdPrintfResize(i*8+(8*6)*1.5, 16*6, white, 1.5, "%d", gpioPinRead(i));
      }

      lcdPrintfRect(0, 0, LCD_WIDTH, LCD_HEIGHT, green, 3, LCD_ALIGN_H_CENTER | LCD_ALIGN_V_BOTTOM,  "ART-PI LCD 시험");

      lcdPrintfRect(0, 0, LCD_WIDTH, LCD_HEIGHT, green, char_size, LCD_ALIGN_H_CENTER | LCD_ALIGN_V_CENTER,  "폰트 크기");
      char_size += 0.1/2;
      if (char_size >= 8)
      {
        char_size = 1.0;
      }

      lcdDrawFillRect(x, 90, 30*2, 30*2, red);
      lcdDrawFillRect(lcdGetWidth()-x, 90+30*2, 30*2, 30*2, green);
      lcdDrawFillRect(x + 30, 90+30*4, 30*2, 30*2, blue);

      lcdDrawFillRect(x, 480-30, 30, 30, red);

      draw_time = millis()-pre_time_draw;

      x += 2;

      x %= lcdGetWidth();
      y %= lcdGetHeight();


      uint8_t touch_cnt;

      touch_cnt = touchGetTouchedCount();
      if (touch_cnt > 0)
      {
        touch_data_t data;

        for (int i=0; i<touch_cnt; i++)
        {
          touchGetTouchedData(i, &data);
          lcdDrawFillRect(data.x-50, data.y-50, 100, 100, green);
        }
      }

      thread->hearbeat++;

      lcdRequestDraw();
    }
    delay(1);
  }

}





