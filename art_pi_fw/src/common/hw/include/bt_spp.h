/*
 * bt_spp.h
 *
 *  Created on: 2021. 5. 8.
 *      Author: baram
 */

#ifndef SRC_HW_DRIVER_BT_SPP_BT_SPP_H_
#define SRC_HW_DRIVER_BT_SPP_BT_SPP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


bool btSppInit(void);
bool btSppIsOpen(void);
bool btSppExcute(void);

uint32_t btSppAvailable(void);
uint8_t  btSppRead(void);
uint32_t btSppWrite(uint8_t *p_data, uint32_t length);


#ifdef __cplusplus
}
#endif


#endif /* SRC_HW_DRIVER_BT_SPP_BT_SPP_H_ */
