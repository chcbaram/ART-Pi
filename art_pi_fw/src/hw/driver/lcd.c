/*
 * lcd.c
 *
 *  Created on: Dec 5, 2020
 *      Author: baram
 */




#include "lcd.h"
#include "gpio.h"
#include "led.h"
#include "spi.h"
#include "ltdc.h"
#include "resize.h"
#include "hangul/PHan_Lib.h"


#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

#define PIXEL_FORMAT_RGB565          0x00000002U   /*!< RGB565 LTDC pixel format   */

#define MAKECOL(r, g, b) ( ((r)<<11) | ((g)<<5) | (b))

#define LCD_OPT_DEF   __attribute__((optimize("O2")))


typedef struct
{
  int16_t x;
  int16_t y;
} lcd_pixel_t;




static bool is_init = false;


static uint8_t backlight_value = 100;
static LcdResizeMode lcd_resize_mode = LCD_RESIZE_NEAREST;

static volatile uint32_t fps_pre_time;
static volatile uint32_t fps_time;
static volatile uint32_t fps_count = 0;

static volatile uint32_t draw_fps = 30;
static volatile uint32_t draw_frame_time = 0;

static void (*available_callback)(void) = NULL;

extern uint16_t *ltdc_draw_buffer;
//static uint16_t *p_draw_frame_buf = NULL;

static uint16_t __attribute__((section(".lcd_buf"))) lcd_buffer[HW_LCD_WIDTH * HW_LCD_HEIGHT];



static void lcdSetup(void);
void disHanFont(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor);
void disHanFontBuffer(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor);
void lcdSwapFrameBuffer(void);
static void lcdDrawLineBuffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, lcd_pixel_t *line);
static void lcdDrawPixelBuffer(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code);
static uint16_t lcdGetColorMix(uint16_t c1, uint16_t c2, uint8_t mix);
void lcdFillBuffer(void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);



void lcdTransferDoneISR(void)
{
  fps_time = millis() - fps_pre_time;
  fps_pre_time = millis();

  if (fps_time > 0)
  {
    fps_count = 1000 / fps_time;
  }
  if (available_callback != NULL)
  {
    available_callback();
  }
}



bool lcdInit(void)
{
  lcdSetup();
  ltdcInit();

  delay(200);
  lcdSetBackLight(100);
  return true;
}

bool lcdDeInit(void)
{
  lcdSetBackLight(0);

  return true;
}



void lcdSetup(void)
{
  // LCD_BL Off
  gpioPinWrite(1, _DEF_LOW);
}


uint8_t lcdGetBackLight(void)
{
  return backlight_value;
}

void lcdSetBackLight(uint8_t value)
{
  value = constrain(value, 0, 100);

  if (value != backlight_value)
  {
    backlight_value = value;
  }

  if (value > 0)
  {
    gpioPinWrite(1, _DEF_HIGH);
  }
  else
  {
    gpioPinWrite(1, _DEF_LOW);
  }
}

uint32_t lcdGetDrawTime(void)
{
  return draw_frame_time;
}

bool lcdIsInit(void)
{
  return is_init;
}

void lcdReset(void)
{
}

void lcdSetAvailableCallback(void (*func)(void))
{
  available_callback = func;
}

LCD_OPT_DEF uint32_t lcdReadPixel(uint16_t x_pos, uint16_t y_pos)
{
  return ltdc_draw_buffer[y_pos * LCD_WIDTH + x_pos];
}

LCD_OPT_DEF void lcdDrawPixel(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code)
{
  ltdc_draw_buffer[y_pos * LCD_WIDTH + x_pos] = rgb_code;
}

LCD_OPT_DEF void lcdDrawPixelMix(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code, uint8_t mix)
{
  uint16_t color1, color2;

  color1 = ltdc_draw_buffer[y_pos * LCD_WIDTH + x_pos];
  color2 = rgb_code;

  ltdc_draw_buffer[y_pos * LCD_WIDTH + x_pos] = lcdGetColorMix(color1, color2, 255-mix);
}

LCD_OPT_DEF void lcdDrawPixelBuffer(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code)
{
  lcd_buffer[y_pos * LCD_WIDTH + x_pos] = rgb_code;
}

LCD_OPT_DEF void lcdClear(uint32_t rgb_code)
{
  lcdClearBuffer(rgb_code);

  lcdUpdateDraw();
}

LCD_OPT_DEF void lcdClearBuffer(uint32_t rgb_code)
{
#if 0
  uint16_t *p_buf = lcdGetFrameBuffer();

  for (int i=0; i<LCD_WIDTH * LCD_HEIGHT; i++)
  {
    p_buf[i] = rgb_code;
  }
#else
  lcdFillBuffer((void *)lcdGetFrameBuffer(), lcdGetWidth(), lcdGetHeight(), 0, rgb_code);
#endif
}

void lcdFillBuffer(void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex)
{
  /* Set up mode */
  DMA2D->CR      = 0x00030000UL | (1 << 9);
  DMA2D->OCOLR   = ColorIndex;

  /* Set up pointers */
  DMA2D->OMAR    = (uint32_t)pDst;

  /* Set up offsets */
  DMA2D->OOR     = OffLine;

  /* Set up pixel format */
  DMA2D->OPFCCR  = PIXEL_FORMAT_RGB565;

  /*  Set up size */
  DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint32_t)ySize;

  DMA2D->CR     |= DMA2D_CR_START;

  /* Wait until transfer is done */
  while (DMA2D->CR & DMA2D_CR_START)
  {
  }
}

void lcdSetDoubleBuffer(bool enable)
{
  ltdcSetDoubleBuffer(enable);
}

bool lcdGetDoubleBuffer(void)
{
  return ltdcGetDoubleBuffer();
}

LCD_OPT_DEF void lcdDrawFillCircle(int32_t x0, int32_t y0, int32_t r, uint16_t color)
{
  int32_t  x  = 0;
  int32_t  dx = 1;
  int32_t  dy = r+r;
  int32_t  p  = -(r>>1);


  lcdDrawHLine(x0 - r, y0, dy+1, color);

  while(x<r)
  {

    if(p>=0) {
      dy-=2;
      p-=dy;
      r--;
    }

    dx+=2;
    p+=dx;

    x++;

    lcdDrawHLine(x0 - r, y0 + x, 2 * r+1, color);
    lcdDrawHLine(x0 - r, y0 - x, 2 * r+1, color);
    lcdDrawHLine(x0 - x, y0 + r, 2 * x+1, color);
    lcdDrawHLine(x0 - x, y0 - r, 2 * x+1, color);
  }
}

LCD_OPT_DEF void lcdDrawCircleHelper( int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x     = 0;

  while (x < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4)
    {
      lcdDrawPixel(x0 + x, y0 + r, color);
      lcdDrawPixel(x0 + r, y0 + x, color);
    }
    if (cornername & 0x2)
    {
      lcdDrawPixel(x0 + x, y0 - r, color);
      lcdDrawPixel(x0 + r, y0 - x, color);
    }
    if (cornername & 0x8)
    {
      lcdDrawPixel(x0 - r, y0 + x, color);
      lcdDrawPixel(x0 - x, y0 + r, color);
    }
    if (cornername & 0x1)
    {
      lcdDrawPixel(x0 - r, y0 - x, color);
      lcdDrawPixel(x0 - x, y0 - r, color);
    }
  }
}

LCD_OPT_DEF void lcdDrawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  // smarter version
  lcdDrawHLine(x + r    , y        , w - r - r, color); // Top
  lcdDrawHLine(x + r    , y + h - 1, w - r - r, color); // Bottom
  lcdDrawVLine(x        , y + r    , h - r - r, color); // Left
  lcdDrawVLine(x + w - 1, y + r    , h - r - r, color); // Right

  // draw four corners
  lcdDrawCircleHelper(x + r        , y + r        , r, 1, color);
  lcdDrawCircleHelper(x + w - r - 1, y + r        , r, 2, color);
  lcdDrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  lcdDrawCircleHelper(x + r        , y + h - r - 1, r, 8, color);
}

LCD_OPT_DEF void lcdDrawFillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -r - r;
  int32_t y     = 0;

  delta++;

  while (y < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }

    y++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1)
    {
      lcdDrawHLine(x0 - r, y0 + y, r + r + delta, color);
      lcdDrawHLine(x0 - y, y0 + r, y + y + delta, color);
    }
    if (cornername & 0x2)
    {
      lcdDrawHLine(x0 - r, y0 - y, r + r + delta, color); // 11995, 1090
      lcdDrawHLine(x0 - y, y0 - r, y + y + delta, color);
    }
  }
}

LCD_OPT_DEF void lcdDrawFillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  // smarter version
  lcdDrawFillRect(x, y + r, w, h - r - r, color);

  // draw four corners
  lcdDrawFillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  lcdDrawFillCircleHelper(x + r, y + r        , r, 2, w - r - r - 1, color);
}

LCD_OPT_DEF void lcdDrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color)
{
  lcdDrawLine(x1, y1, x2, y2, color);
  lcdDrawLine(x1, y1, x3, y3, color);
  lcdDrawLine(x2, y2, x3, y3, color);
}

LCD_OPT_DEF void lcdDrawFillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color)
{
  uint16_t max_line_size_12 = max(abs(x1-x2), abs(y1-y2));
  uint16_t max_line_size_13 = max(abs(x1-x3), abs(y1-y3));
  uint16_t max_line_size_23 = max(abs(x2-x3), abs(y2-y3));
  uint16_t max_line_size = max_line_size_12;
  uint16_t i = 0;

  if (max_line_size_13 > max_line_size)
  {
    max_line_size = max_line_size_13;
  }
  if (max_line_size_23 > max_line_size)
  {
    max_line_size = max_line_size_23;
  }

  lcd_pixel_t line[max_line_size];

  lcdDrawLineBuffer(x1, y1, x2, y2, color, line);
  for (i = 0; i < max_line_size_12; i++)
  {
    lcdDrawLine(x3, y3, line[i].x, line[i].y, color);
  }
  lcdDrawLineBuffer(x1, y1, x3, y3, color, line);
  for (i = 0; i < max_line_size_13; i++)
  {
    lcdDrawLine(x2, y2, line[i].x, line[i].y, color);
  }
  lcdDrawLineBuffer(x2, y2, x3, y3, color, line);
  for (i = 0; i < max_line_size_23; i++)
  {
    lcdDrawLine(x1, y1, line[i].x, line[i].y, color);
  }
}

uint32_t lcdGetFps(void)
{
  return fps_count;
}

uint32_t lcdGetFpsTime(void)
{
  return fps_time;
}

bool lcdDrawAvailable(void)
{
  return ltdcDrawAvailable();
}

bool lcdRequestDraw(void)
{
  ltdcRequestDraw();

  return true;
}

void lcdUpdateDraw(void)
{
  lcdRequestDraw();
  while(lcdDrawAvailable() != true)
  {
    delay(1);
  }
}

void lcdSetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{

}

void lcdSwapFrameBuffer(void)
{

}

uint16_t *lcdGetFrameBuffer(void)
{
  return (uint16_t *)ltdcGetFrameBuffer();
}

uint16_t *lcdGetCurrentFrameBuffer(void)
{
  return (uint16_t *)ltdcGetCurrentFrameBuffer();
}

void lcdDisplayOff(void)
{
}

void lcdDisplayOn(void)
{
  lcdSetBackLight(lcdGetBackLight());
}

int32_t lcdGetWidth(void)
{
  return LCD_WIDTH;
}

int32_t lcdGetHeight(void)
{
  return LCD_HEIGHT;
}


void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      lcdDrawPixel(y0, x0, color);
    } else
    {
      lcdDrawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawLineBuffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, lcd_pixel_t *line)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      if (line != NULL)
      {
        line->x = y0;
        line->y = x0;
      }
      lcdDrawPixel(y0, x0, color);
    } else
    {
      if (line != NULL)
      {
        line->x = x0;
        line->y = y0;
      }
      lcdDrawPixel(x0, y0, color);
    }
    if (line != NULL)
    {
      line++;
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  lcdDrawLine(x, y, x, y+h-1, color);
}

void lcdDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  lcdDrawLine(x, y, x+w-1, y, color);
}

void lcdDrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  for (int16_t i=x; i<x+w; i++)
  {
    lcdDrawVLine(i, y, h, color);
  }
}

void lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawHLine(x, y, w, color);
  lcdDrawHLine(x, y+h-1, w, color);
  lcdDrawVLine(x, y, h, color);
  lcdDrawVLine(x+w-1, y, h, color);
}

void lcdDrawFillScreen(uint16_t color)
{
  lcdDrawFillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, color);
}

void lcdPrintf(int x, int y, uint16_t color,  const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i, x_Pre = x;
  PHAN_FONT_OBJ FontBuf;
  uint8_t font_width;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );

    disHanFont( x, y, &FontBuf, color);

    Size_Char = FontBuf.Size_Char;
    if (Size_Char >= 2)
    {
      font_width = 16;
      x += 2*8;
    }
    else
    {
      font_width = 8;
      x += 1*8;
    }

    if ((x+font_width) > HW_LCD_WIDTH)
    {
      x  = x_Pre;
      y += 16;
    }

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }
}

void lcdPrintfResize(int x, int y, uint16_t color,  float ratio, const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i;
  int x_Pre = x;
  int y_Pre = y;
  PHAN_FONT_OBJ FontBuf;
  uint8_t font_width;
  resize_image_t r_src, r_dst;
  uint16_t pixel;

  r_src.x = 0;
  r_src.y = 0;
  r_src.w = 0;
  r_src.h = 16;
  r_src.stride = LCD_WIDTH;
  r_src.p_data = lcd_buffer;


  x = 0;
  y = 0;

  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );


    disHanFontBuffer(x, y, &FontBuf, 0xFF);

    Size_Char = FontBuf.Size_Char;
    if (Size_Char >= 2)
    {
      font_width = 16;
      x += 2*8;
    }
    else
    {
      font_width = 8;
      x += 1*8;
    }

    r_src.w += font_width;

    if ((x+font_width) > HW_LCD_WIDTH)
    {
      x  = x_Pre;
      y += 16;
      r_src.h += 16;
    }

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }

  r_dst.x = 0;
  r_dst.y = r_src.y + r_src.h;
  r_dst.w = r_src.w * ratio;
  r_dst.h = r_src.h * ratio;
  r_dst.stride = LCD_WIDTH;
  r_dst.p_data = lcd_buffer;

  if (lcd_resize_mode == LCD_RESIZE_BILINEAR)
  {
    resizeImageFastGray(&r_src, &r_dst);

    for (int i_y=0; i_y<r_dst.h; i_y++)
    {
      for (int i_x=0; i_x<r_dst.w; i_x++)
      {
        pixel = lcd_buffer[(i_y+r_dst.y)*LCD_WIDTH + i_x];
        if (pixel > 0)
        {
          lcdDrawPixelMix(x_Pre+i_x, y_Pre+i_y, color, pixel);
        }
      }
    }
  }
  else
  {
    resizeImageNearest(&r_src, &r_dst);

    for (int i_y=0; i_y<r_dst.h; i_y++)
    {
      for (int i_x=0; i_x<r_dst.w; i_x++)
      {
        pixel = lcd_buffer[(i_y+r_dst.y)*LCD_WIDTH + i_x];
        if (pixel > 0)
        {
          lcdDrawPixel(x_Pre+i_x, y_Pre+i_y, color);
        }
      }
    }
  }
}

void lcdPrintfRect(int x, int y, int w, int h, uint16_t color, float ratio, uint16_t align, const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i;
  int x_Pre = x;
  int y_Pre = y;
  PHAN_FONT_OBJ FontBuf;
  uint8_t font_width;
  resize_image_t r_src, r_dst;
  uint16_t pixel;

  r_src.x = 0;
  r_src.y = 0;
  r_src.w = 0;
  r_src.h = 16;
  r_src.stride = LCD_WIDTH;
  r_src.p_data = lcd_buffer;

  x = 0;
  y = 0;

  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );


    disHanFontBuffer(x, y, &FontBuf, 0xFF);

    Size_Char = FontBuf.Size_Char;
    if (Size_Char >= 2)
    {
      font_width = 16;
      x += 2*8;
    }
    else
    {
      font_width = 8;
      x += 1*8;
    }

    r_src.w += font_width;

    if ((x+font_width) > HW_LCD_WIDTH)
    {
      x  = x_Pre;
      y += 16;
      r_src.h += 16;
    }

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }

  r_dst.x = 0;
  r_dst.y = r_src.y + r_src.h;
  r_dst.w = r_src.w * ratio;
  r_dst.h = r_src.h * ratio;
  r_dst.stride = LCD_WIDTH;
  r_dst.p_data = lcd_buffer;


  if (lcd_resize_mode == LCD_RESIZE_BILINEAR)
  {
    resizeImageFastGray(&r_src, &r_dst);
  }
  else
  {
    resizeImageNearest(&r_src, &r_dst);
  }

  int x_o = x_Pre;
  int y_o = y_Pre;


  if (w > r_dst.w)
  {
    if (align & LCD_ALIGN_H_CENTER)
    {
      x_o += (w-r_dst.w)/2;
    }
    if (align & LCD_ALIGN_H_RIGHT)
    {
      x_o += (w-r_dst.w);
    }
  }
  if (h > r_dst.h)
  {
    if (align & LCD_ALIGN_V_CENTER)
    {
      y_o += (h-r_dst.h)/2 + 0;
    }
    if (align & LCD_ALIGN_V_BOTTOM)
    {
      y_o += (h-r_dst.h);
    }
  }

  if (lcd_resize_mode == LCD_RESIZE_BILINEAR)
  {
    for (int i_y=0; i_y<r_dst.h; i_y++)
    {
      for (int i_x=0; i_x<r_dst.w; i_x++)
      {
        pixel = lcd_buffer[(i_y+r_dst.y)*LCD_WIDTH + i_x];
        if (pixel > 0)
        {
          lcdDrawPixelMix(x_o+i_x, y_o+i_y, color, pixel);
        }
      }
    }
  }
  else
  {
    for (int i_y=0; i_y<r_dst.h; i_y++)
    {
      for (int i_x=0; i_x<r_dst.w; i_x++)
      {
        pixel = lcd_buffer[(i_y+r_dst.y)*LCD_WIDTH + i_x];
        if (pixel > 0)
        {
          lcdDrawPixel(x_o+i_x, y_o+i_y, color);
        }
      }
    }
  }
}

void lcdSetResizeMode(LcdResizeMode mode)
{
  lcd_resize_mode = mode;
}

uint32_t lcdGetStrWidth(const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i;
  PHAN_FONT_OBJ FontBuf;
  uint32_t str_len;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  str_len = 0;

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );

    Size_Char = FontBuf.Size_Char;

    str_len += (Size_Char * 8);

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }

  return str_len;
}

void disHanFont(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      uint8_t font_data;

      font_data = FontPtr->FontBuffer[i*FontSize +j];

      for( Loop=0; Loop<8; Loop++ )
      {
        if( (font_data<<Loop) & (0x80))
        {
          lcdDrawPixel(x + index_x, y + i, textcolor);
        }
        index_x++;
      }
    }
  }
}

void disHanFontBuffer(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  if (textcolor == 0)
  {
    textcolor = 1;
  }
  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      uint8_t font_data;

      font_data = FontPtr->FontBuffer[i*FontSize +j];

      for( Loop=0; Loop<8; Loop++ )
      {
        if(font_data & ((uint8_t)0x80>>Loop))
        {
          lcdDrawPixelBuffer(x + index_x, y + i, textcolor);
        }
        else
        {
          lcdDrawPixelBuffer(x + index_x, y + i, 0);
        }
        index_x++;
      }
    }
  }
}

LCD_OPT_DEF uint16_t lcdGetColorMix(uint16_t c1_, uint16_t c2_, uint8_t mix)
{
  uint16_t r, g, b;
  uint16_t ret;
  uint16_t c1;
  uint16_t c2;

#if 0
  c1 = ((c1_>>8) & 0x00FF) | ((c1_<<8) & 0xFF00);
  c2 = ((c2_>>8) & 0x00FF) | ((c2_<<8) & 0xFF00);
#else
  c1 = c1_;
  c2 = c2_;
#endif
  r = ((uint16_t)((uint16_t) GETR(c1) * mix + GETR(c2) * (255 - mix)) >> 8);
  g = ((uint16_t)((uint16_t) GETG(c1) * mix + GETG(c2) * (255 - mix)) >> 8);
  b = ((uint16_t)((uint16_t) GETB(c1) * mix + GETB(c2) * (255 - mix)) >> 8);

  ret = MAKECOL(r, g, b);



  //return ((ret>>8) & 0xFF) | ((ret<<8) & 0xFF00);;
  return ret;
}


#ifdef _USE_HW_CMDIF


void lcdCmdif(void)
{
  bool ret = true;
  uint32_t pre_time;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("test", 0) == true)
  {
    //uint16_t line_buf[HW_LCD_WIDTH];

    for (int i=0; i<HW_LCD_WIDTH; i++)
    {
      //line_buf[i] = red;
    }

    pre_time = millis();
    /*
    lcd.setWindow(_win_x, _win_y, (_win_x+_win_w)-1, (_win_y+_win_h)-1);
    for (int i=0; i<HW_LCD_HEIGHT; i++)
    {
      lcd.sendBuffer((uint8_t *)line_buf, HW_LCD_WIDTH*2, 10);
    }
    */
    lcdDrawFillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, red);
    lcdRequestDraw();
    cmdifPrintf("%d ms\n", millis()-pre_time);
  }
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("info", 0) == true)
  {
    uint8_t info[4] = {0, };
    cmdifPrintf("%X %X %X %X\n", info[0], info[1], info[2], info[3]);
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "lcd test \n");
  }
}
#endif






