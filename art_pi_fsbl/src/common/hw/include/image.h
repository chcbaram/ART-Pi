/*
 * image.h
 *
 *  Created on: 2021. 5. 2.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_IMAGE_H_
#define SRC_COMMON_HW_INCLUDE_IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "def.h"
#include "lvgl/lvgl.h"




typedef struct
{
  const image_res_t *p_res;
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} image_t;




#define IMAGE_RES_DEF(var_name) extern image_res_t var_name;


image_t imageLoad(const image_res_t *p_image_res);
image_t imageCreate(const image_res_t *p_image_res, int16_t x, int16_t y, int16_t w, int16_t h);


bool imageDraw(const image_t *p_image, int16_t x, int16_t y);


#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_HW_INCLUDE_IMAGE_H_ */
