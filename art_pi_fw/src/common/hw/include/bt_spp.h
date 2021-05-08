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
bool btSppExcute(void);

#ifdef __cplusplus
}
#endif


#endif /* SRC_HW_DRIVER_BT_SPP_BT_SPP_H_ */
