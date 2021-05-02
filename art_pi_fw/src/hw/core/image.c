/*
 * image.c
 *
 *  Created on: 2021. 5. 2.
 *      Author: baram
 */




#include "image.h"
#include "lcd.h"



image_t imageLoad(const image_res_t *p_image_res)
{
  image_t ret;

  ret.p_res = p_image_res;
  ret.x = 0;
  ret.y = 0;
  ret.w = p_image_res->header.w;
  ret.h = p_image_res->header.h;

  return ret;
}

image_t imageCreate(const image_res_t *p_image_res, int16_t x, int16_t y, int16_t w, int16_t h)
{
  image_t ret;

  ret.p_res = p_image_res;
  ret.x = x;
  ret.y = y;
  ret.w = w;
  ret.h = h;

  return ret;
}

__attribute__((optimize("O2"))) bool imageDraw(const image_t *p_image, int16_t x, int16_t y)
{
  uint16_t color;
  int16_t  o_x;
  int16_t  o_y;
  uint16_t *p_data;
  int16_t  stride;

  stride = p_image->p_res->header.w;
  p_data = (uint16_t *)p_image->p_res->data;

  for (int iy=0; iy<p_image->h; iy++)
  {
    o_y = (p_image->y + iy) * stride;
    for (int ix=0; ix<p_image->w; ix++)
    {
      o_x   = p_image->x + ix;

      color = p_data[o_y + o_x];

      if (color != green)
      {
        lcdDrawPixel(x+ix, y+iy, color);
      }
    }
  }

  return true;
}
