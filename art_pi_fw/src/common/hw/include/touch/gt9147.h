/*
 * gt9147.h
 *
 *  Created on: 2021. 4. 29.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_TOUCH_GT9147_H_
#define SRC_COMMON_HW_INCLUDE_TOUCH_GT9147_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_GT9147


#include "touch.h"


bool gt9147InitDriver(touch_driver_t *p_driver);




#ifdef __cplusplus
}
#endif


#endif



#endif /* SRC_COMMON_HW_INCLUDE_TOUCH_GT9147_H_ */
