/**
 * @file dali_dexal.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief Functions performing DEXAL-specific DALI sequences
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "dali_dexal.h"
#include "dali_MemoryBank.h"

#define MEMBANK_DEXAL_POWER  30
#define MEMBANK_DEXAL_STATS  29
#define MEMBANK_DEXAL_ENERGY 36

#define INDEX_DEXAL_POWER           6
#define INDEX_DEXAL_LAMPCOUNT       5
#define INDEX_DEXAL_CURRENTCASETEMP 8
#define INDEX_DEXAL_MAXCASETEMP     9
#define INDEX_DEXAL_LAMPCOUNT       5
#define INDEX_DEXAL_TOTALNRG        5

#define SIZE_DEXAL_POWER     3
#define SIZE_DEXAL_LAMPCOUNT 3
#define SIZE_DEXAL_CASETEMP  1
#define SIZE_DEXAL_ENERGY    4
#define DEXAL_POWER_UNIT  ((float).015625f)
#define DEXAL_NRG_UNIT    ((float)1.0f)


_Bool getDexalPowerRaw(uint8_t addr, uint32_t *pPwr)
{
  static uint8_t aPwr[SIZE_DEXAL_POWER];
  if(true == daliReadMemoryBank(evShortAddress     ,
                                addr               ,
                                MEMBANK_DEXAL_POWER,
                                INDEX_DEXAL_POWER  ,
                                SIZE_DEXAL_POWER   ,
                                &aPwr[0]           ))
  {
    *pPwr = ((aPwr[0]<<16) + (aPwr[1]<<8) + aPwr[2]);
    return true;
  }
  return false;
}

_Bool getDexalPowerFloat(uint8_t addr, float fPwrUnit, float * pfPwr)
{
  uint32_t rawPwr_l = 0;
  if(true == getDexalPowerRaw(addr, &rawPwr_l))
  { 
    *pfPwr = rawPwr_l*DEXAL_POWER_UNIT;
    return true;
  }
  return false;
} 

_Bool getDexalLampRunTime(uint8_t addr, uint32_t * pRunTime )
{
  static uint8_t aRunTime[SIZE_DEXAL_LAMPCOUNT];
  if(true == daliReadMemoryBank(evShortAddress       ,
                                addr                 ,
                                MEMBANK_DEXAL_STATS  ,
                                INDEX_DEXAL_LAMPCOUNT,
                                SIZE_DEXAL_LAMPCOUNT ,
                                &aRunTime[0]         ))
  {
    *pRunTime = ((aRunTime[0])<<16) + ((aRunTime[1])<<8) +aRunTime[2];
    return true;
  }
  return false;
} 

_Bool getDexalMaxCaseTemp(uint8_t addr, uint8_t  * pMaxTemp)
{
  if(true == daliReadMemoryBank(evShortAddress         ,
                                addr                   ,
                                MEMBANK_DEXAL_STATS    ,
                                INDEX_DEXAL_MAXCASETEMP,
                                SIZE_DEXAL_CASETEMP    ,
                                pMaxTemp               ))
  {  
    return true;
  }
  return false;
} 


_Bool getDexalCurrentCaseTemp(uint8_t addr, uint8_t  * pCurTemp)
{
  if(true == daliReadMemoryBank(evShortAddress             ,  
                                addr                       ,
                                MEMBANK_DEXAL_STATS        ,
                                INDEX_DEXAL_CURRENTCASETEMP,
                                SIZE_DEXAL_CASETEMP        ,
                                pCurTemp                   ))
  {
    return true;
  }
  return false;
} 


_Bool getDexalEnergyRaw(uint8_t addr, uint64_t * pTotNRG)
{
  static uint8_t aNRG[SIZE_DEXAL_ENERGY];
  if(true == daliReadMemoryBank(evShortAddress      ,
                                addr                ,
                                MEMBANK_DEXAL_ENERGY,
                                INDEX_DEXAL_TOTALNRG,
                                SIZE_DEXAL_ENERGY   ,
                                &aNRG[0]            ))
  {
    *pTotNRG = ((aNRG[0])<<24) + ((aNRG[1])<<16) + ((aNRG[2])<<8) + (aNRG[3]);
    return true;
  }
  return false;
} 

float getDexalPowerUnitFloat(void)
{
  return DEXAL_POWER_UNIT;
}


float getDexalEnergyUnitFloat(void)
{
  return  DEXAL_NRG_UNIT;
}