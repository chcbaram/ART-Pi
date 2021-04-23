/**
  ******************************************************************************
  * @file    Loader_Src.h
  * @author  MCD Tools Team
  * @date    October-2015
  * @brief   Loader Header file.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOADER_SRC_H
#define __LOADER_SRC_H

/* Includes ------------------------------------------------------------------*/
#include "hw.h"



#ifdef __ICCARM__                 //IAR
#define KeepInCompilation __root
#elif __CC_ARM                    //MDK-ARM
#define KeepInCompilation __attribute__((used))
#else // TASKING                  //TrueStudio
#define KeepInCompilation __attribute__((used))
#endif


/* Private function prototypes -----------------------------------------------*/
int Init (void);
KeepInCompilation int Write (uint32_t Address, uint32_t Size, uint8_t* buffer);
KeepInCompilation int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress);
KeepInCompilation int Verify (uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size);







#endif /* __LOADER_SRC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
