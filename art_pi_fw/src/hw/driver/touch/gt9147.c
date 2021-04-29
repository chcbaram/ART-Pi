/*
 * gt9147.c
 *
 *  Created on: 2021. 4. 29.
 *      Author: baram
 */




#include "touch/gt9147.h"

#ifdef _USE_HW_GT9147
#include "i2c.h"



#define GT9147_I2C_ADDR             0x5D
#define GT9147_MAX_TOUCHES          5

#define GT9147_REG_TD_STATUS        0x814E
#define GT9147_REG_TOUCH1_XL        0x814F

#define GT9147_ID_G_CHIPID          0x8140


static uint8_t i2c_ch = _DEF_I2C1;
static bool is_init = false;


#ifdef _USE_HW_RTOS
static osMutexId mutex_id;
#endif


typedef struct
{
  uint8_t ID;
  uint8_t XL;
  uint8_t XH;
  uint8_t YL;
  uint8_t YH;
  uint8_t WL;
  uint8_t WH;
  uint8_t Reserved;
} touch_point_t;


static touch_point_t point[GT9147_MAX_TOUCHES];


static bool    init(void);
static uint8_t getTouchedCount(void);
static bool    getTouchedData(uint8_t index, touch_data_t *p_data);
static bool    readReg(uint16_t addr, uint8_t *p_data, uint32_t length);
static bool    writeReg(uint16_t addr, uint8_t *p_data, uint32_t length);




bool gt9147InitDriver(touch_driver_t *p_driver)
{
  p_driver->init = init;
  p_driver->getTouchedCount = getTouchedCount;
  p_driver->getTouchedData = getTouchedData;

  return true;
}



bool init(void)
{
  uint8_t data = 0;
  bool ret = false;


  if (i2cIsInit() != true) return false;

  if (i2cIsBegin(i2c_ch) != true)
  {
    i2cBegin(i2c_ch, 400);
  }

  if (readReg(GT9147_ID_G_CHIPID, &data, 1) == true)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }


  is_init = ret;

#ifdef _USE_HW_RTOS
  osMutexDef(mutex_id);
  mutex_id = osMutexCreate (osMutex(mutex_id));
#endif

  return ret;
}

uint8_t getTouchedCount(void)
{
  uint8_t ret = 0;
  uint8_t data;

#ifdef _USE_HW_RTOS
  osMutexWait(mutex_id, osWaitForever);
#endif

  if (readReg(GT9147_REG_TD_STATUS, &data, 1) == true)
  {
    ret = data & 0x0F;

    if (ret > GT9147_MAX_TOUCHES)
    {
      ret = GT9147_MAX_TOUCHES;
    }

    for (int i=0; i<ret; i++)
    {
      readReg(GT9147_REG_TOUCH1_XL + i*8, (uint8_t *)&point[i], 8);
    }

    uint8_t cmd = 0;
    writeReg(GT9147_REG_TD_STATUS, &cmd, 1);
  }

#ifdef _USE_HW_RTOS
  osMutexRelease(mutex_id);
#endif
  return ret;
}

bool getTouchedData(uint8_t index, touch_data_t *p_data)
{
  bool ret = false;


  if (index >= GT9147_MAX_TOUCHES)
  {
    return false;
  }

#ifdef _USE_HW_RTOS
  osMutexWait(mutex_id, osWaitForever);
#endif


  p_data->event = 0;
  p_data->id    = point[index].ID;

  p_data->x  = point[index].XL << 0;
  p_data->x |= point[index].XH << 8;
  p_data->y  = point[index].YL << 0;
  p_data->y |= point[index].YH << 8;
  p_data->w  = point[index].WL << 0;
  p_data->w |= point[index].WH << 8;

  ret = true;


#ifdef _USE_HW_RTOS
  osMutexRelease(mutex_id);
#endif
  return ret;
}


bool readReg(uint16_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret;

  ret = i2cRead16Bytes(i2c_ch, GT9147_I2C_ADDR, addr, p_data, length, 10);

  return ret;
}

bool writeReg(uint16_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret;

  ret = i2cWrite16Bytes(i2c_ch, GT9147_I2C_ADDR, addr, p_data, length, 10);

  return ret;
}





#endif
