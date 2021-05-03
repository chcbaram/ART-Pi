/*
 * ltdc.h
 *
 *  Created on: Apr 24, 2021
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_LTDC_H_
#define SRC_COMMON_HW_INCLUDE_LTDC_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_LTDC


bool ltdcInit(void);
bool ltdcDrawAvailable(void);
void ltdcRequestDraw(void);
void ltdcSetAlpha(uint16_t LayerIndex, uint32_t value);
uint16_t *ltdcGetFrameBuffer(void);
uint16_t *ltdcGetCurrentFrameBuffer(void);
int32_t  ltdcWidth(void);
int32_t  ltdcHeight(void);
uint32_t ltdcGetBufferAddr(uint8_t index);
bool ltdcLayerInit(uint16_t LayerIndex, uint32_t Address);
void ltdcSetDoubleBuffer(bool enable);
bool ltdcGetDoubleBuffer(void);


#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_LTDC_H_ */
