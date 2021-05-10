/*
 * btstack_port.h
 *
 *  Created on: 2021. 5. 10.
 *      Author: baram
 */

#ifndef SRC_HW_DRIVER_BTSTACK_BTSTACK_PORT_H_
#define SRC_HW_DRIVER_BTSTACK_BTSTACK_PORT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


void btstackPortInit(void);
void btstackPortExecute(void);


#ifdef __cplusplus
}
#endif


#endif /* SRC_HW_DRIVER_BTSTACK_BTSTACK_PORT_H_ */
