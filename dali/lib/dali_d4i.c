/**
 * @file dali_d4i.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief Functions performing D4I-specific DALI sequences
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "dali_d4i.h"
#include "dali_MemoryBank.h"
#include "float.h"
#include "math.h"

#define MEMBANK_D4I_POWER       202/*!< D4i memory bank for power and energy   */
#define INDEX_D4I_POWER        0x0C/*!< Offset into memory bank for power      */
#define INDEX_D4I_ENERGY       0x05/*!< Offset into memory bank for energy     */
#define INDEX_D4I_POWER_SCALE  0x0B/*!< Offset into memory bank for power unit */
#define INDEX_D4I_ENERGY_SCALE 0x04/*!< Offset into memory bank for energy unit*/

#define SIZE_D4I_POWER        4/*!< Power reading size, number bytes to read  */
#define SIZE_D4I_ENERGY       6/*!< Energy reading size, number bytes to read*/
#define SIZE_D4I_POWER_SCALE  1/*!< Power reading unit size, number bytes to read*/
#define SIZE_D4I_ENERGY_SCALE 1/*!< Power reading unit size, number bytes to read*/


#define MEMBANK_D4I_LIGHT_DIAGNOSTIC 206
#define INDEX_LIGHT_SRC_VOLTAGE      0x12
#define INDEX_LIGHT_SRC_CURRENT      0x14

#define SIZE_LIGHT_SRC_VOLTAGE       2
#define SIZE_LIGHT_SRC_CURRENT       2


#define MEMBANK_D4I_GEAR_DIAGNOSTIC 205
#define INDEX_GEAR_TEMP             0x1b

#define SIZE_GEAR_TEMP             1

/**
 * @brief queries the D4i driver for its power unit (2's complement) and returns it as raw data
 * 
 * @param addr 
 * @param pRawPwrUnit 
 * @return _Bool Returns true when transaction complete
 */
_Bool getD4iPowerUnitRaw      (uint8_t addr  , int8_t   * pRawPwrUnit  );

/**
 * @brief queries the D4i driver for its energy unit (2's complement) and returns it as raw data
 * 
 * @param addr 
 * @param pRawNrgUnit 
 * @return _Bool Returns true when transaction complete
 */
_Bool getD4iEnergyUnitRaw     (uint8_t addr  , int8_t   * pRawNrgUnit  );

/**
 * @brief Reads power unit and converts to float
 * 
 * @param addr 
 * @param pFloatPwrUnit 
 * @return _Bool Returns true when transaction complete
 */
_Bool getD4iPowerUnitFloat    (uint8_t addr  , float    * pFloatPwrUnit);

/**
 * @brief Reads energy unit and converts to float
 * 
 * @param addr 
 * @param pFloatNrgUnit 
 * @return _Bool Returns true when transaction complete
 */
_Bool getD4iEnergyUnitFloat   (uint8_t addr  , float    * pFloatNrgUnit);

/**
 * @brief Convert D4i unit to floating point multiplier
 * 
 * @param rawUnit 
 * @param pFloatUnit 
 * @return _Bool 
 */
_Bool convertD4iRawUnitToFloat(int8_t rawUnit, float *pFloatUnit       );



_Bool getD4iUnits(sDaliDriverData_t * psDaliDriverData)
{
  static uint8_t getUnitState = 0;
  switch(getUnitState)
  {
    case 0:
      if(true == getD4iPowerUnitFloat(psDaliDriverData->sStaticData.addr       ,
                                      &psDaliDriverData->sStaticData.fPowerUnit))
      {
        getD4iEnergyUnitFloat(psDaliDriverData->sStaticData.addr        ,
                              &psDaliDriverData->sStaticData.fEnergyUnit);
        getUnitState = 1;
      }
    break;
    case 1:
      if(true == getD4iEnergyUnitFloat(psDaliDriverData->sStaticData.addr        ,
                                       &psDaliDriverData->sStaticData.fEnergyUnit))
      {
        getUnitState = 0;
        return true;
      }
    break;
    default:
    break;
  }
  return false;
}


_Bool getD4iPowerUnitRaw(uint8_t addr, int8_t * pRawPwrUnit)
{
    return daliReadMemoryBank(evShortAddress       ,
                              addr                 ,
                              MEMBANK_D4I_POWER    ,
                              INDEX_D4I_POWER_SCALE,
                              SIZE_D4I_POWER_SCALE ,
                              pRawPwrUnit          );
}


_Bool getD4iEnergyUnitRaw(uint8_t addr, int8_t * pRawNrgUnit)
{
  return daliReadMemoryBank(evShortAddress        ,
                            addr                  ,
                            MEMBANK_D4I_POWER     ,
                            INDEX_D4I_ENERGY_SCALE,
                            SIZE_D4I_POWER_SCALE  ,
                            pRawNrgUnit           );
}


_Bool getD4iEnergyUnitFloat(uint8_t addr, float *pFloatNrgUnit)
{
  int8_t nrgUnit_l;
  if(true == getD4iEnergyUnitRaw(addr, &nrgUnit_l))
  {
    convertD4iRawUnitToFloat(nrgUnit_l,pFloatNrgUnit);
    return true;
  }
  return false;
}


_Bool getD4iPowerUnitFloat(uint8_t addr, float *pFloatPwrUnit)
{
  int8_t pwrUnit_l;
  if(true == getD4iPowerUnitRaw(addr,&pwrUnit_l))
  {
      convertD4iRawUnitToFloat(pwrUnit_l, pFloatPwrUnit);
      return true;
  }
  return false;
}

_Bool getD4iPowerRaw(uint8_t addr, uint32_t * pRawPwr)
{
    static uint8_t aPwr[SIZE_D4I_POWER];
    if(true == daliReadMemoryBank(evShortAddress   ,
                                  addr             ,
                                  MEMBANK_D4I_POWER,
                                  INDEX_D4I_POWER  ,
                                  SIZE_D4I_POWER   ,
                                  &aPwr[0]         ))
    {//cannot simply return ReadMemoryBank because the bytes need reordering
        *(pRawPwr) = (aPwr[0]<<24) + (aPwr[1]<<16) + (aPwr[2]<<8) + (aPwr[3]);
        return true;
    }
    return false;
}

_Bool getD4iEnergyRaw(uint8_t addr, uint64_t *pNrgRaw)
{
  typedef union
  {
        uint8_t  aNrg[SIZE_D4I_ENERGY];
        uint64_t u64Nrg               ;
  }uD4iNrg_t;

  uint8_t byteSwapCtr = 0;
  static uint8_t aNrg[SIZE_D4I_ENERGY] = {0};
         uD4iNrg_t uD4iNrg = {0};

  if(true == daliReadMemoryBank(evShortAddress   ,
                                addr             ,
                                MEMBANK_D4I_POWER,
                                INDEX_D4I_ENERGY ,
                                SIZE_D4I_ENERGY  ,
                                &aNrg[0]         ))
  {
    while(byteSwapCtr < SIZE_D4I_ENERGY)
    {
      uD4iNrg.aNrg[byteSwapCtr] = aNrg[SIZE_D4I_ENERGY - (byteSwapCtr+1)];
      byteSwapCtr++;
    }

    *(pNrgRaw) =  uD4iNrg.u64Nrg;
    return true;
  }
}


_Bool convertD4iRawUnitToFloat(int8_t rawUnit, float *pFloatUnit)
{
    switch(rawUnit)
    {
      case -6:
        *pFloatUnit = 0.000001f;
      break;
      case -5:
        *pFloatUnit = 0.00001f;
      break;
      case -4:
        *pFloatUnit = 0.0001f;
      break;
      case -3:
        *pFloatUnit = 0.001f;
      break;
      case -2:
        *pFloatUnit = 0.01f;
      break;
      case -1:
        *pFloatUnit = 0.1f;
      break;
      case 0:
        *pFloatUnit = 1.0f;
      break;
      case 1:
        *pFloatUnit = 10.0f;
      break;
      case 2:
        *pFloatUnit = 100.0f;
      break;
      case 3:
        *pFloatUnit = 1000.0f;
      break;
      case 4:
        *pFloatUnit = 10000.0f;
      break;
      case 5:
        *pFloatUnit = 100000.0f;
      break;
      case 6:
        *pFloatUnit = 100000.0f;
      break;
      default:
        *pFloatUnit = 0.0f;
        return false;
      break;
    }
    return true;
}

_Bool getD4iPowerFloat(uint8_t addr, float floatUnit, float * pFloatPwr)
{
  uint32_t rawPwr_l = 0;
  if(true == getD4iPowerRaw(addr,&rawPwr_l))
  {
    *pFloatPwr = floatUnit*rawPwr_l;
    return true;
  }
  return false;
}


_Bool getMB202(uint8_t addr, uint8_t * dataPtr)
{
  return daliReadMemoryBank(evShortAddress,addr,202,0,SIZE_MB_202,dataPtr);
}

_Bool getMB203(uint8_t addr, uint8_t * dataPtr)
{
  return daliReadMemoryBank(evShortAddress,addr,203,0,SIZE_MB_203,dataPtr);
}

_Bool getMB204(uint8_t addr, uint8_t *dataPtr)
{
  return daliReadMemoryBank(evShortAddress,addr,204,0,SIZE_MB_204,dataPtr);
}

_Bool getMB205(uint8_t addr, uint8_t *dataPtr)
{
  return daliReadMemoryBank(evShortAddress,addr,205,0,SIZE_MB_205,dataPtr);
}

_Bool getMB206(uint8_t addr, uint8_t *dataPtr)
{
  return daliReadMemoryBank(evShortAddress,addr,206,0,SIZE_MB_206,dataPtr);
}

_Bool getMB207(uint8_t addr, uint8_t *dataPtr)
{
 return daliReadMemoryBank(evShortAddress,addr,207,0,SIZE_MB_207,dataPtr);
}

_Bool getMB1(uint8_t addr, uint8_t *dataPtr)
{
 return daliReadMemoryBank(evShortAddress,addr,1,0,SIZE_MB_1,dataPtr);
}

_Bool getD4iMemBanks(uint8_t addr, uint8_t *dataPtr)
{
    static uint8_t getD4iMemBanksState = 0;
    switch(getD4iMemBanksState)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
    }
}

_Bool getD4iLightSrcVoltage(uint8_t addr, uint16_t *pVolts)
{
    _Bool done = false;
    uint8_t swap0 = 0;
    uint8_t swap1 = 0;
    done =  daliReadMemoryBank(evShortAddress,addr,MEMBANK_D4I_LIGHT_DIAGNOSTIC,
                               INDEX_LIGHT_SRC_VOLTAGE,SIZE_LIGHT_SRC_VOLTAGE,pVolts);
    if(done == true)
    {
        swap0 = (*pVolts) & 0xff;
        swap1 = ((*pVolts) & 0xff00)>>8;
        *pVolts = swap1 + (swap0<<8);
    }
}

_Bool getD4iLightSrcCurrent(uint8_t addr, uint16_t *pAmps)
{
    _Bool done = false;
    uint8_t swap0 = 0;
    uint8_t swap1 = 0;
    done =  daliReadMemoryBank(evShortAddress,addr,MEMBANK_D4I_LIGHT_DIAGNOSTIC,
                               INDEX_LIGHT_SRC_CURRENT,SIZE_LIGHT_SRC_CURRENT,pAmps);
    if(done == true)
    {
        swap0 = (*pAmps) & 0xff;
        swap1 = ((*pAmps) & 0xff00)>>8;
        *pAmps = swap1 + (swap0<<8);
    }
}

_Bool getD4iGearTemperature(uint8_t addr, uint16_t *pTemp)
{
        return daliReadMemoryBank(evShortAddress,addr,MEMBANK_D4I_GEAR_DIAGNOSTIC,
                                  INDEX_GEAR_TEMP,SIZE_GEAR_TEMP,pTemp);
}

