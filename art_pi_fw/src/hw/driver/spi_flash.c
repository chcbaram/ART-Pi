/*
 * spi_flash.c
 *
 *  Created on: 2021. 5. 4.
 *      Author: baram
 */




#include "spi_flash.h"
#include "qspi/w25q64fv.h"
#include "gpio.h"
#include "cli.h"


#define SPI_CS_L()    gpioPinWrite(_PIN_GPIO_SPI_FLASH_CS, _DEF_LOW)
#define SPI_CS_H()    gpioPinWrite(_PIN_GPIO_SPI_FLASH_CS, _DEF_HIGH)


static bool is_init = false;


static SPI_HandleTypeDef hspi1;



static bool spiFlashTransfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout);
static bool spiFlashGetID(uint8_t *p_id_tbl, uint32_t lenght);
static bool spiFlashWriteEnable(void);
static bool spiFlashWaitBusy(uint32_t timeout);


#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif



bool spiFlashInit(void)
{
  bool ret = true;
  uint8_t id_tbl[4];

  ret = spiFlashReset();

  if (spiFlashGetID(id_tbl, 4) == true)
  {
    if (id_tbl[1] == 0xEF && id_tbl[2] == 0x40 && id_tbl[3] == 0x18)
    {
      logPrintf("W25Q64JV         \t: OK\r\n");
      ret = true;
    }
    else
    {
      logPrintf("W25Q64JV         \t: Fail %X %X %X\r\n", id_tbl[1], id_tbl[2], id_tbl[3]);
      ret = false;
    }
  }
  else
  {
    logPrintf("spiFlash            \t: Fail\r\n");
    ret = false;
  }

  is_init = ret;

#ifdef _USE_HW_CLI
  cliAdd("spiFlash", cliCmd);
#endif

  return ret;
}

bool spiFlashIsInit(void)
{
  return is_init;
}

bool spiFlashReset(void)
{
  bool ret = true;


  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; // 30Mbps
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;

  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    return false;
  }


  uint8_t tx_buf[4];


  //-- Reset Memory
  //
  tx_buf[0] = RESET_ENABLE_CMD;

  SPI_CS_L();
  ret &= spiFlashTransfer(tx_buf, NULL, 1, 10);
  SPI_CS_H();

  tx_buf[0] = RESET_MEMORY_CMD;
  SPI_CS_L();
  ret &= spiFlashTransfer(tx_buf, NULL, 1, 10);
  SPI_CS_H();

  delay(10);
  if (ret != true) return false;



  return ret;
}

bool spiFlashRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t tx_buf[4];

  tx_buf[0] = READ_CMD;
  tx_buf[1] = addr >> 16;
  tx_buf[2] = addr >> 8;
  tx_buf[3] = addr >> 0;

  SPI_CS_L();
  ret &= spiFlashTransfer(tx_buf, NULL, 4, 10);
  if (ret == true)
  {
    ret &= spiFlashTransfer(NULL, p_data, length, 500);
  }
  SPI_CS_H();

  return ret;
}

bool spiFlashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t tx_buf[4];
  uint32_t end_addr, current_size, current_addr;


  if (addr+length > spiFlashGetLength())
  {
    return false;
  }


  /* Calculation of the size between the write address and the end of the page */
  current_size = W25Q64FV_PAGE_SIZE - (addr % W25Q64FV_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > length)
  {
    current_size = length;
  }

  /* Initialize the adress variables */
  current_addr = addr;
  end_addr = addr + length;


  /* Perform the write page by page */
  do
  {
    //-- Write Enable
    //
    if (spiFlashWriteEnable() == false)
    {
      return false;
    }

    //-- Write Page
    //
    SPI_CS_L();
    tx_buf[0] = PAGE_PROG_CMD;
    tx_buf[1] = current_addr >> 16;
    tx_buf[2] = current_addr >> 8;
    tx_buf[3] = current_addr >> 0;
    ret &= spiFlashTransfer(tx_buf, NULL, 4, 10);

    ret &= spiFlashTransfer(p_data, NULL, current_size, 10);
    SPI_CS_H();


    ret &= spiFlashWaitBusy(2000);
    if (ret == false)
    {
      break;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    p_data += current_size;
    current_size = ((current_addr + W25Q64FV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q64FV_PAGE_SIZE;
  } while (current_addr < end_addr);


  return ret;
}

bool spiFlashErase(uint32_t addr, uint32_t length)
{
  bool ret = true;
  uint32_t flash_length;
  uint32_t block_size;
  uint32_t block_begin;
  uint32_t block_end;
  uint32_t i;



  flash_length = W25Q64FV_FLASH_SIZE;
  block_size   = W25Q64FV_SUBSECTOR_SIZE;


  if ((addr > flash_length) || ((addr+length) > flash_length))
  {
    return false;
  }
  if (length == 0)
  {
    return false;
  }


  block_begin = addr / block_size;
  block_end   = (addr + length - 1) / block_size;


  for (i=block_begin; i<=block_end; i++)
  {
    ret = spiFlashEraseSector(block_size*i);
    if (ret == false)
    {
      break;
    }
  }

  return ret;
}

bool spiFlashEraseBlock(uint32_t block_addr)
{
  bool ret = true;
  uint8_t tx_buf[4];


  //-- Write Enable
  //
  if (spiFlashWriteEnable() == false)
  {
    return false;
  }


  //-- Erase Sector
  //
  SPI_CS_L();
  tx_buf[0] = SECTOR_ERASE_CMD;
  tx_buf[1] = block_addr >> 16;
  tx_buf[2] = block_addr >> 8;
  tx_buf[3] = block_addr >> 0;
  ret &= spiFlashTransfer(tx_buf, NULL, 4, 10);
  SPI_CS_H();

  ret &= spiFlashWaitBusy(W25Q64FV_SECTOR_ERASE_MAX_TIME);

  return ret;
}

bool spiFlashEraseSector(uint32_t sector_addr)
{
  bool ret = true;
  uint8_t tx_buf[4];


  //-- Write Enable
  //
  if (spiFlashWriteEnable() == false)
  {
    return false;
  }


  //-- Erase Sector
  //
  SPI_CS_L();
  tx_buf[0] = SUBSECTOR_ERASE_CMD;
  tx_buf[1] = sector_addr >> 16;
  tx_buf[2] = sector_addr >> 8;
  tx_buf[3] = sector_addr >> 0;
  ret &= spiFlashTransfer(tx_buf, NULL, 4, 10);
  SPI_CS_H();

  ret &= spiFlashWaitBusy(W25Q64FV_SUBSECTOR_ERASE_MAX_TIME);

  return ret;
}

uint32_t spiFlashGetAddr(void)
{
  return 0;
}

uint32_t spiFlashGetLength(void)
{
  return W25Q64FV_FLASH_SIZE;
}

bool spiFlashEraseChip(void);
bool spiFlashGetStatus(void);
bool spiFlashGetInfo(spi_flash_info_t* p_info);

bool spiFlashGetID(uint8_t *p_id_tbl, uint32_t length)
{
  bool ret = true;

  uint8_t tx_buf[4];
  uint8_t rx_buf[4];

  //-- Read ID
  //
  tx_buf[0] = READ_ID_CMD;
  SPI_CS_L();
  ret &= spiFlashTransfer(tx_buf, rx_buf, 4, 10);
  SPI_CS_H();
  if (ret == true)
  {
    if (length > 4)
    {
      length = 4;
    }
    for (int i=0; i<length; i++)
    {
      p_id_tbl[i] = rx_buf[i];
    }
  }
  return ret;
}

bool spiFlashWriteEnable(void)
{
  bool ret = true;
  uint8_t tx_buf[4];
  uint8_t rx_buf[4];
  uint32_t pre_time;


  //-- Write Enable
  //
  SPI_CS_L();
  tx_buf[0] = WRITE_ENABLE_CMD;
  ret &= spiFlashTransfer(tx_buf, NULL, 1, 10);
  SPI_CS_H();

  pre_time = millis();
  while(1)
  {
    SPI_CS_L();
    tx_buf[0] = READ_STATUS_REG_CMD;
    ret &= spiFlashTransfer(tx_buf, rx_buf, 2, 10);
    SPI_CS_H();

    if (ret == true)
    {
      if (rx_buf[1] & W25Q64FV_SR_WREN)
      {
        break;
      }
    }
    if (millis()-pre_time >= 100)
    {
      ret = false;
      break;
    }
  }

  return ret;
}

bool spiFlashWaitBusy(uint32_t timeout)
{
  bool ret = true;
  uint32_t pre_time;
  uint8_t tx_buf[2];
  uint8_t rx_buf[2];


  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= timeout)
    {
      ret = false;
      break;
    }

    SPI_CS_L();
    tx_buf[0] = READ_STATUS_REG_CMD;
    ret &= spiFlashTransfer(tx_buf, rx_buf, 2, 10);
    SPI_CS_H();

    if (ret == true)
    {
      uint8_t busy_bit;

      busy_bit = rx_buf[1] & W25Q64FV_SR_WIP;
      if (busy_bit == 0)
      {
        break;
      }
    }
  }


  return ret;
}

bool spiFlashTransfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout)
{
  bool ret = true;
  HAL_StatusTypeDef status;


  if (rx_buf == NULL)
  {
    status =  HAL_SPI_Transmit(&hspi1, tx_buf, length, timeout);
  }
  else if (tx_buf == NULL)
  {
    status =  HAL_SPI_Receive(&hspi1, rx_buf, length, timeout);
  }
  else
  {
    status =  HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, length, timeout);
  }

  if (status != HAL_OK)
  {
    ret = false;
  }

  return ret;
}


void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PB5     ------> SPI1_MOSI
    PG9     ------> SPI1_MISO
    PA5     ------> SPI1_SCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PB5     ------> SPI1_MOSI
    PG9     ------> SPI1_MISO
    PA5     ------> SPI1_SCK
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
}





#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = true;
  uint32_t i;
  uint32_t addr;
  uint32_t length;
  uint8_t  data;
  uint32_t pre_time;
  bool flash_ret;


  if (args->argc == 1)
  {
    if(args->isStr(0, "info") == true)
    {
      cliPrintf("flash addr  : 0x%X\n", 0x0000000);
    }
    else
    {
      ret = false;
    }
  }
  else if (args->argc == 3)
  {
    if(args->isStr(0, "read") == true)
    {
      addr   = (uint32_t)args->getData(1);
      length = (uint32_t)args->getData(2);

      for (i=0; i<length; i++)
      {
        flash_ret = spiFlashRead(addr+i, &data, 1);

        if (flash_ret == true)
        {
          cliPrintf( "addr : 0x%X\t 0x%02X\n", addr+i, data);
        }
        else
        {
          cliPrintf( "addr : 0x%X\t Fail\n", addr+i);
        }
      }
    }
    else if(args->isStr(0, "erase") == true)
    {
      addr   = (uint32_t)args->getData(1);
      length = (uint32_t)args->getData(2);

      pre_time = millis();
      flash_ret = spiFlashErase(addr, length);

      cliPrintf( "addr : 0x%X\t len : %d %d ms\n", addr, length, (millis()-pre_time));
      if (flash_ret)
      {
        cliPrintf("OK\n");
      }
      else
      {
        cliPrintf("FAIL\n");
      }
    }
    else if(args->isStr(0, "write") == true)
    {
      addr = (uint32_t)args->getData(1);
      data = (uint8_t )args->getData(2);

      pre_time = millis();
      flash_ret = spiFlashWrite(addr, &data, 1);

      cliPrintf( "addr : 0x%X\t 0x%02X %dms\n", addr, data, millis()-pre_time);
      if (flash_ret)
      {
        cliPrintf("OK\n");
      }
      else
      {
        cliPrintf("FAIL\n");
      }
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }


  if (ret == false)
  {
    cliPrintf( "spiFlash info\n");
    cliPrintf( "spiFlash read  [addr] [length]\n");
    cliPrintf( "spiFlash erase [addr] [length]\n");
    cliPrintf( "spiFlash write [addr] [data]\n");
  }

}
#endif





