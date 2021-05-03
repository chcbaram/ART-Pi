/*
 * touch.h
 *
 *  Created on: 2021. 4. 29.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_TOUCH_H_
#define SRC_COMMON_HW_INCLUDE_TOUCH_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"



#ifdef _USE_HW_TOUCH


typedef struct
{
  uint8_t  event;
  uint8_t  id;
  int16_t  x;
  int16_t  y;
  int16_t  w;
} touch_data_t;



typedef struct touch_driver_t_ touch_driver_t;

typedef struct touch_driver_t_
{
  bool     (*init)(void);
  uint8_t  (*getTouchedCount)(void);
  bool     (*getTouchedData)(uint8_t index, touch_data_t *p_data);
} touch_driver_t;


bool    touchInit(void);
uint8_t touchGetTouchedCount(void);
bool    touchGetTouchedData(uint8_t index, touch_data_t *p_data);


#endif


#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_TOUCH_H_ */
