/*
 * ap_def.h
 *
 *  Created on: 2021. 5. 1.
 *      Author: baram
 */

#ifndef SRC_AP_AP_DEF_H_
#define SRC_AP_AP_DEF_H_


#include "hw.h"



typedef enum
{
  EVENT_SDCARD_CONNECTED,
  EVENT_SDCARD_DISCONNECTED,
  EVENT_TOUCH_PRESSED,
  EVENT_TOUCH_RELEASED,
  EVENT_BT_OPEN,
  EVENT_BT_CLOSE
} Event_t;



typedef struct thread_t_ threat_t;

typedef struct thread_t_
{
  bool is_init;
  bool is_start;

  const char *name;
  uint32_t freq;
  uint32_t hearbeat;

  threat_t *list;

  bool (*notify)(Event_t event);


  bool (*onEvent)(Event_t event);

} thread_t;


typedef enum
{
  THREAD_ID_DISPLAY,
  THREAD_ID_INFO,
  THREAD_ID_CLI,
  THREAD_ID_BTSTACK,
  THREAD_ID_MAX
} ThreadId_t;








#endif /* SRC_AP_AP_DEF_H_ */
