/**
 * @file dali_power.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "dali_power.h"
#include "dali_d4i.h"
#include "dali_dexal.h"
#include "dali_sr.h"



_Bool readDALIPower(sDaliDriverData_t * psDaliDriverStaticData, uint32_t *pPwr)
{
  switch(psDaliDriverStaticData->sStaticData.eDaliType)
  {
  case evD4i:
    if(true == getD4iPowerRaw(psDaliDriverStaticData->sStaticData.addr,pPwr))
    {
      return true;
    }
  break;
  case evDexal:
    if(true == getDexalPowerRaw(psDaliDriverStaticData->sStaticData.addr,pPwr))
    {
      return true;
    }
  break;
  case evSR:
    /*Get power reading from SR driver.  Functions handle unlocking if needed
      As a result, the first read of a pssword protected item may take longer
      than normal.*/
      if(true == getSRPowerRaw(psDaliDriverStaticData->sStaticData.addr      ,
                                 pPwr                                        ))
      {
        return true;
      }
 
  break;
  case evDali:
  default:
    *pPwr = 4294967295;//Power reporting not supported by DALI driver.
  return true;
  break;
  }
  return false;
}


_Bool readDALIEnergy(sDaliDriverData_t * psDaliDriverStaticData, uint64_t *pNrg)
{
  switch(psDaliDriverStaticData->sStaticData.eDaliType)
  {
  case evD4i:
    if(true == getD4iEnergyRaw(psDaliDriverStaticData->sStaticData.addr,pNrg))
    {
      return true;
    }
  break;
  case evDexal:
    if(true == getDexalEnergyRaw(psDaliDriverStaticData->sStaticData.addr,pNrg))
    {
      return true;
    }
  break;
  case evSR:
    if(true == getSREnergyRaw(psDaliDriverStaticData->sStaticData.addr,pNrg))
    {
      return true;
    }
  break;
  case evDali:
  default:
    *pNrg = 0xFFFFFFFFFFFF;//Energy reporting not supported by DALI driver.
  return true;
  break;
  }
  return false;
}


_Bool readDALIPowerFloat(sDaliDriverData_t * psDaliDriverStaticData, float * pfPwr)
{
  switch(psDaliDriverStaticData->sStaticData.eDaliType)
  {
  case evD4i:
    if(true == getD4iPowerFloat(psDaliDriverStaticData->sStaticData.addr      ,
                                psDaliDriverStaticData->sStaticData.fPowerUnit,
                                pfPwr                                         ))
    {
      return true;
    }
  break;
  case evDexal:
    if(true == getDexalPowerFloat(psDaliDriverStaticData->sStaticData.addr      ,
                                  psDaliDriverStaticData->sStaticData.fPowerUnit,
                                  pfPwr                                         ))
    {
      return true;
    }
  break;
  case evSR:
    if(true == getSRPowerFloat(psDaliDriverStaticData->sStaticData.addr      ,
                               psDaliDriverStaticData->sStaticData.fPowerUnit,
                               pfPwr                                         ))
    {
      return true;
    }
  break;
  case evDali:
  default:
    *pfPwr = -1.0f;//Power reporting not supported by DALI driver.
  return true;
  break;
  }
  return false;
}