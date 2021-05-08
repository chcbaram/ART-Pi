/*
 * bt_port.h
 *
 *  Created on: 2021. 5. 8.
 *      Author: baram
 */

#ifndef SRC_HW_DRIVER_BT_SPP_BT_PORT_H_
#define SRC_HW_DRIVER_BT_SPP_BT_PORT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


void btPortInit(void);
void btPortExecute(void);


#ifdef __cplusplus
}
#endif

#endif /* SRC_HW_DRIVER_BT_SPP_BT_PORT_H_ */
