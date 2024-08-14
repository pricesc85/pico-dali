/**
 * @file dali_sr.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dali_MemoryBank.h"
#include "dali_sr.h"
#include "dali_maxDeviceSupport.h"


#define MEMBANK_SR_POWER           68
#define MEMBANK_SR_NRG             68
#define INDEX_MEMBANK_SR_PWR_DATA_START  4
#define INDEX_SR_POWER             17
#define INDEX_SR_POWER_UNIT        16
#define INDEX_SR_ENERGY             4
#define INDEX_SR_ENERGY_UNIT       21
#define INDEX_SR_ENERGY_RESETTABLE 10
#define INDEX_SR_ENERGY_UNIT_RST   22
#define SIZE_SR_POWER               4
#define SIZE_SR_NRG                 6
#define SIZE_SR_NRG_RST             6
#define SIZE_SR_POWER_MEMBANK      23


//#define SR_POWER_UNIT    ((float)0.1f)


#define SR_PASSWORD     0x7D,0x8D,0xDE,0x7F
#define SR_ACCESS_LEVEL 0x5A
#define SR_SET_COMMAND  1
#define SR_CRC          0x3E

#define SR_PASSWORD_MEMBANK 67
#define SR_PASSWORD_INDEX    7


uint32_t srLockStatus[2] = {0};/*!<Encodes lock status of SR drivers. 0 is locked (default at poweron).*/

/**
 * @brief sets lock status to unlocked for driver at address, returns false if address is out of bounds
 * 
 * @param address 
 * @return _Bool  returns false if address is out of bounds.
 */
_Bool    setSRLockStatus(uint8_t address);

/**
 * @brief Converts the raw 2's complement exponent for 10^x stored in the driver to a floating point multiplier
 * 
 * @param unit 
 * @return float 
 */
float    srUnitToFloat  (uint8_t unit   );

_Bool getSRLockStatus(uint8_t address)
{
  if(address < 32)
  {
      if(0 == ((srLockStatus[0]>>address) & 1))
      {
          return true;
      }
  }
  else if(address < 64)
  {
      if(0 == ((srLockStatus[1]>>(address - 32)) & 1))
      {
          return true;
      }
  }
  return false;
}


_Bool setSRLockStatus(uint8_t address)
{
  if(address < 32)
  {
    srLockStatus[0] |= (1<<address);
    return true;
  }
  else if(address < 64)//largest dali short address
  {
    srLockStatus[1] |= (1<<(address - 32));  
    return true;
  }
  return false;
}

_Bool srUnlockPowerReading(uint8_t address)
{
  const uint8_t aSRPwd[] = {SR_PASSWORD,SR_ACCESS_LEVEL,SR_SET_COMMAND,SR_CRC};
  _Bool bUnlockStatus = 0;
  bUnlockStatus = daliUnlockWriteLockMemoryBank(evShortAddress       ,
                                                address              ,
                                                SR_PASSWORD_MEMBANK  ,
                                                SR_PASSWORD_INDEX    ,
                                                sizeof(aSRPwd)       ,
                                                (uint8_t *)&aSRPwd[0]);
   if(bUnlockStatus == 1)
   {
    setSRLockStatus(address);
    return true;
   }
   return false;
}

_Bool getSRPowerFloat(uint8_t address, float fPwrUnit, float * fPwr)
{
  uint32_t rawPwr_l;
  switch(getSRLockStatus(address))
  {
    case true://locked
      srUnlockPowerReading(address);
    break;
    case false://unlocked
      if(true == getSRPowerRaw(address, &rawPwr_l))
      {
        *(fPwr) = rawPwr_l * fPwrUnit;
        return true;
      }
    break;
  }
  return false;
}

_Bool getSRPowerRaw(uint8_t address, uint32_t *pwr)
{
  static uint8_t aPwr[SIZE_SR_POWER] = {0};

  if(true == daliReadMemoryBank(evShortAddress  ,
                                address         , 
                                MEMBANK_SR_POWER,
                                INDEX_SR_POWER  ,
                                SIZE_SR_POWER   ,
                                &aPwr[0]        ))
  {
    *pwr = (aPwr[0]<<24) + (aPwr[1]<<16) + (aPwr[2]<<8) + (aPwr[3]<<0);
    return true;
  }
  return false;
}

_Bool getSREnergyRaw(uint8_t address, uint64_t *nrg)
{
  typedef union
  {
        uint8_t  aNrg[SIZE_SR_NRG];
        uint64_t u64Nrg           ;
  }uSRNrg_t;
  
  uint8_t  byteSwapCtr             =  0 ;
  uSRNrg_t uSRNrg                  = {0};
  static uint8_t aNrg[SIZE_SR_NRG] = {0};

  switch(getSRLockStatus(address))
  {
  case true://locked
   srUnlockPowerReading(address);//will update lock status when complete
  break;
  case false://unlocked
    if(true == daliReadMemoryBank(evShortAddress ,
                                  address        ,
                                  MEMBANK_SR_NRG ,
                                  INDEX_SR_ENERGY,
                                  SIZE_SR_NRG    ,
                                  &aNrg[0]       ))
    {
     while(byteSwapCtr < SIZE_SR_NRG)
      {
        uSRNrg.aNrg[byteSwapCtr] = aNrg[SIZE_SR_NRG - (byteSwapCtr + 1)];
        byteSwapCtr++;
      }
      *nrg = uSRNrg.u64Nrg;
      return true;
    }
  break;
  }
  return false;
}

_Bool getSRUnits(sDaliDriverData_t * psDaliDriverData)
{

  static uint8_t srMembankContents[SIZE_SR_POWER_MEMBANK - INDEX_MEMBANK_SR_PWR_DATA_START] = {0};
  _Bool readStatus = false;
  switch(getSRLockStatus(psDaliDriverData->sStaticData.addr))
  {
  case true://locked
    srUnlockPowerReading(psDaliDriverData->sStaticData.addr);
  break;
  case false://unlocked
    readStatus = daliReadMemoryBank(evShortAddress                                         ,
                                    psDaliDriverData->sStaticData.addr                     ,
                                    MEMBANK_SR_POWER                                       ,
                                    INDEX_MEMBANK_SR_PWR_DATA_START                        ,
                                    SIZE_SR_POWER_MEMBANK - INDEX_MEMBANK_SR_PWR_DATA_START,
                                    srMembankContents                                      );
    if(true == readStatus)
    {
      psDaliDriverData->sStaticData.fPowerUnit     = srUnitToFloat(srMembankContents[INDEX_SR_POWER_UNIT      - INDEX_MEMBANK_SR_PWR_DATA_START]);
      psDaliDriverData->sStaticData.fEnergyUnit    = srUnitToFloat(srMembankContents[INDEX_SR_ENERGY_UNIT     - INDEX_MEMBANK_SR_PWR_DATA_START]);
      psDaliDriverData->sStaticData.fRSTEnergyUnit = srUnitToFloat(srMembankContents[INDEX_SR_ENERGY_UNIT_RST - INDEX_MEMBANK_SR_PWR_DATA_START]);
    }
  break;
  }
  return readStatus;
}

float srUnitToFloat(uint8_t unit)
{
  switch(unit)
  {
    case 0://reasonable
      return 1.0f;
    break;
    case 1://less reasonable
      return 10.0f;
    break;
    case 2://even less reasonable
      return 100.0f;
    break;
    case 3://even less reasonable
      return 1000.0f;
    break;
    case 4://and so on
      return 10000.0f;
    break;
    case 5://the sun
      return 100000.0f;
    break;
    case 0xff:
      return 0.1f;
    break;
    case 0xfe:
      return 0.01f;
    break;
    case 0xfd:
      return 0.001f;
    break;
    case 0xfc:
      return 0.0001f;
    break;
    case 0xfb:
      return 0.00001f;
    break;
    case 0xfa:
      return 0.000001f;
    break;
    default:
    break;
  } 
  return 0.0f;//something went wrong
}
