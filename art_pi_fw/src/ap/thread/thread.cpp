/*
 * thread.cpp
 *
 *  Created on: 2021. 5. 1.
 *      Author: baram
 */




#include "thread.h"
#include "common/info.h"
#include "common/cli.h"
#include "display/display.h"


static const char *thread_name = "Dummy       ";

static thread_t thread_list[THREAD_ID_MAX];



bool threadInit(void)
{
  bool ret = true;


  for (int i=0; i<THREAD_ID_MAX; i++)
  {
    thread_list[i].list = &thread_list[0];
    thread_list[i].name = thread_name;

    thread_list[i].is_init = false;

    thread_list[i].freq = 0;
    thread_list[i].hearbeat = 0;
  }

  ret &= infoThreadInit(&thread_list[THREAD_ID_INFO]);
  ret &= cliThreadInit(&thread_list[THREAD_ID_CLI]);
  ret &= displayThreadInit(&thread_list[THREAD_ID_DISPLAY]);


  return ret;
}
