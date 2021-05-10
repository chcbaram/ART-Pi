/*
 * display.cpp
 *
 *  Created on: 2021. 5. 1.
 *      Author: baram
 */




#include "display.h"


IMAGE_RES_DEF(image_test);


static const char *thread_name = "display     ";
static thread_t *thread = NULL;
static bool is_sdcard = false;
static bool is_bt = false;

static void displayThread(void const *argument);
static bool displayEvent(Event_t event);




bool displayThreadInit(thread_t *p_thread)
{
  bool ret = false;

  thread = p_thread;

  thread->name = thread_name;
  thread->onEvent = displayEvent;

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
  bool is_touch = false;
  bool is_touch_pre = false;
  image_t image1;
  image_t image2;

  thread->is_start = true;


  image1 = imageLoad(&image_test);
  image2 = imageCreate(&image_test, 19, 13, 30, 28);


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

      imageDraw(&image1, x + 30, 90+30*5);
      imageDraw(&image2, x + 30, 90+30*6);

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
        is_touch = true;
      }
      else
      {
        is_touch = false;
      }

      if (is_touch != is_touch_pre)
      {
        if (is_touch)
        {
          thread->notify(EVENT_TOUCH_PRESSED);
        }
        else
        {
          thread->notify(EVENT_TOUCH_RELEASED);
        }
      }

      is_touch_pre = is_touch;

      if (is_sdcard == true)
      {
        lcdPrintfRect(0, 0, LCD_WIDTH, 32, green, 2, LCD_ALIGN_H_RIGHT | LCD_ALIGN_V_TOP,  "[SD카드]");
      }
      if (is_bt == true)
      {
        lcdPrintfRect(0, 32, LCD_WIDTH, 32, green, 2, LCD_ALIGN_H_RIGHT | LCD_ALIGN_V_TOP,  "[BT_SPP]");
      }

      thread->hearbeat++;

      lcdRequestDraw();
    }
    delay(1);
  }

}

bool displayEvent(Event_t event)
{
  bool ret = true;


  switch(event)
  {
    case EVENT_SDCARD_CONNECTED:
      logPrintf("EVENT_SDCARD_CONNECTED : %s,%d\n", __FILE__, __LINE__);
      is_sdcard = true;
      break;

    case EVENT_SDCARD_DISCONNECTED:
      logPrintf("EVENT_SDCARD_DISCONNECTED : %s,%d\n", __FILE__, __LINE__);
      is_sdcard = false;
      break;

    case EVENT_BT_OPEN:
      logPrintf("EVENT_BT_OPEN : %s,%d\n", __FILE__, __LINE__);
      is_bt = true;
      break;

    case EVENT_BT_CLOSE:
      logPrintf("EVENT_BT_CLOSE : %s,%d\n", __FILE__, __LINE__);
      is_bt = false;
      break;

    default:
      ret = false;
      break;
  }

  return ret;
}




