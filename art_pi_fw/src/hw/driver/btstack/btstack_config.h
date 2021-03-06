/*
 * btstack_config.h
 *
 *  Created on: 2021. 5. 8.
 *      Author: baram
 */

#ifndef SRC_HW_DRIVER_BT_SPP_BTSTACK_CONFIG_H_
#define SRC_HW_DRIVER_BT_SPP_BTSTACK_CONFIG_H_


// Port related features
#define HAVE_EMBEDDED_TIME_MS


// BTstack features that can be enabled
#define ENABLE_CLASSIC
#define ENABLE_PRINTF_HEXDUMP
//#define ENABLE_LOG_INFO


// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE        1021
#define MAX_NR_BNEP_CHANNELS        MAX_SPP_CONNECTIONS
#define MAX_NR_BNEP_SERVICES        1
#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES  2
#define MAX_NR_GATT_CLIENTS 1
#define MAX_NR_HCI_CONNECTIONS      MAX_SPP_CONNECTIONS
#define MAX_NR_HFP_CONNECTIONS      0
#define MAX_NR_L2CAP_CHANNELS       (1+MAX_SPP_CONNECTIONS)
#define MAX_NR_L2CAP_SERVICES       2
#define MAX_NR_LE_DEVICE_DB_ENTRIES 0
#define MAX_NR_RFCOMM_CHANNELS      MAX_SPP_CONNECTIONS
#define MAX_NR_RFCOMM_MULTIPLEXERS  MAX_SPP_CONNECTIONS
#define MAX_NR_RFCOMM_SERVICES      1
#define MAX_NR_SERVICE_RECORD_ITEMS 1
#define MAX_NR_SM_LOOKUP_ENTRIES    3
#define MAX_NR_WHITELIST_ENTRIES    1
#define MAX_SPP_CONNECTIONS         1


#define NVM_NUM_LINK_KEYS           16


#endif /* SRC_HW_DRIVER_BT_SPP_BTSTACK_CONFIG_H_ */
