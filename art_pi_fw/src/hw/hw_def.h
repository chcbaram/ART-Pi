/*
 * hw_def.h
 *
 *  Created on: Dec 6, 2020
 *      Author: baram
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"



#define _HW_DEF_RTOS_MEM_SIZE(x)              ((x)/4)


#define _HW_DEF_RTOS_THREAD_PRI_MAIN          osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_CLI           osPriorityNormal
#define _HW_DEF_RTOS_THREAD_PRI_LED           osPriorityNormal


#define _HW_DEF_RTOS_THREAD_MEM_MAIN          _HW_DEF_RTOS_MEM_SIZE( 6*1024)
#define _HW_DEF_RTOS_THREAD_MEM_CLI           _HW_DEF_RTOS_MEM_SIZE( 6*1024)
#define _HW_DEF_RTOS_THREAD_MEM_LED           _HW_DEF_RTOS_MEM_SIZE(    256)




#define _USE_HW_SD
#define _USE_HW_QSPI
#define _USE_HW_FLASH
#define _USE_HW_FATFS
#define _USE_HW_TOUCH
#define _USE_HW_GT9147
#define _USE_HW_RTOS


#define _USE_HW_LED
#define      HW_LED_MAX_CH          2

#define _USE_HW_UART
#define      HW_UART_MAX_CH         1

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    24
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       1

#define _USE_HW_SDRAM
#define      HW_SDRAM_MEM_ADDR      0xC0000000
#define      HW_SDRAM_MEM_SIZE      (32*1024*1024)

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         4

#define _USE_HW_LCD
#define _USE_HW_LTDC
#define      HW_LCD_WIDTH           800
#define      HW_LCD_HEIGHT          480

#define _USE_HW_LOG
#define      HW_LOG_CH              _DEF_UART1





#define _PIN_GPIO_SDCARD_DETECT     0



#endif /* SRC_HW_HW_DEF_H_ */
