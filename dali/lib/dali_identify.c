/**
 * @file dali_identify.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "dali.h"
#include "dali_MemoryBank.h"
#include "dali_identify.h"
#include "dali_dexal.h"
#include "dali_sr.h"
#include "dali_d4i.h"

#include <string.h>

typedef struct
{
    sGTIN_t OTi30DX;
    sGTIN_t OTi50DX;
    sGTIN_t OTi85DX;
}sDexalGtinTable_t;

typedef struct
{
  sGTIN_t XI040C110V054VPT1;
  sGTIN_t XI040C110V054VPT2;
  sGTIN_t XI075C200V054VPT1;
}sSrGtinTable_t;

typedef struct
{
  sDexalGtinTable_t sDexal;
  sSrGtinTable_t    sSR   ;
}sSupportedGtinTable_t;

typedef union
{
  sGTIN_t saGTIN[sizeof(sSupportedGtinTable_t)/sizeof(sGTIN_t)];
  sSupportedGtinTable_t sGtinTable;
}uGTIN_t;

uGTIN_t uGTIN = 
{
  .sGtinTable = 
  {
    .sDexal = 
    {
      .OTi30DX = {0x00,0x0A,0xBD,0xE8,0x23,0xEC},//real data
      .OTi50DX = {0x00,0x0A,0xBD,0xE8,0x23,0xFD},//real data
      .OTi85DX = {0x03,0xAF,0xA3,0xA3,0x5D,0xF1} //real data
    },
    .sSR = 
    {
      .XI040C110V054VPT1 = {0x00,0xb5,0xdc,0x6b,0xdd,0x00},//real data
      .XI040C110V054VPT2 = {0x00,0xb5,0xdc,0x6c,0x2f,0x1b},//real data
      .XI075C200V054VPT1 = {1,2,3,4,5,6} //dummy data...I don't have one of these TODO: get unit and fill in
    }
  }
};

/**
 * @brief Compare the GTIN against a list of GTINs to determine DALI type
 * 
 */
void  verifyModelByGTIN         (sDaliDriverData_t  *);

/**
 * @brief Read membank0 of a driver, store the data to psDaliDriverData and ID the driver
 * 
 */
_Bool identifyDaliDriver(sDaliDriverData_t *psDaliDriverData);


_Bool identifyDaliDriver(sDaliDriverData_t *psDaliDriverData)
{
  if(true == daliReadMemoryBank(evShortAddress                     ,
                                psDaliDriverData->sStaticData.addr ,//control gear address to read
                                0                                    ,//memory bank
                                3                                    ,//index of first byte to read
                                sizeof(sMemoryBank0_t)               ,//num bytes to read 
                                &psDaliDriverData->sMemBnk0.gtin[0])
    )
  {
      verifyModelByGTIN(psDaliDriverData);
      return true;
  }
  return false;
 }
  
_Bool identifyDaliDrivers(saDaliNetworkData_t *psaDaliNetworkData)
{
  static uint8_t ctrlGearIndex     = 0;
  static uint8_t identifyState     = 0;

  switch(identifyState)
  {
    case 0:
      while(ctrlGearIndex < psaDaliNetworkData->numDrivers)
      {
        if(ctrlGearIndex >= MAX_SUPPORTED_DRIVERS)
        {
          //TODO: Add some notification of more than MAX_SUPPORTED_DRIVERS on bus
          ctrlGearIndex = 0;
          return true;
        }
        psaDaliNetworkData->uData[ctrlGearIndex].sData.sStaticData.addr = ctrlGearIndex;

        ctrlGearIndex++;
      }
      ctrlGearIndex = 0;
      identifyState = 1;
    case 1:
      if(true == identifyDaliDriver(&psaDaliNetworkData->uData[ctrlGearIndex].sData))
      {
        if(psaDaliNetworkData->uData[ctrlGearIndex].sData.sStaticData.eDaliType == evD4i)
        {
          identifyState = 2;//Get D4i reporting units, could vary by model
          break;
        }
        else if(psaDaliNetworkData->uData[ctrlGearIndex].sData.sStaticData.eDaliType == evSR)
        {
          identifyState = 3;//Get SR reporting units, could vary by model
          break;
        }
        ctrlGearIndex++;
        if(ctrlGearIndex >= psaDaliNetworkData->numDrivers)
        {
          identifyState = 0;
          ctrlGearIndex = 0;
          return true;
        }
      }
    break;
    case 2://get reporting units for D4i.  
    if(true == getD4iUnits(&psaDaliNetworkData->uData[ctrlGearIndex].sData))
    {
      ctrlGearIndex++;
      if(ctrlGearIndex >= psaDaliNetworkData->numDrivers)
      {
        identifyState = 0;
        ctrlGearIndex = 0;
        return true;
      }
      identifyState = 1;
    }
    break;
    case 3://get reporting units for SR.
    if(true == getSRUnits(&psaDaliNetworkData->uData[ctrlGearIndex].sData))
    {
      ctrlGearIndex++;
      if(ctrlGearIndex >= psaDaliNetworkData->numDrivers)
      {
        identifyState = 0;
        ctrlGearIndex = 0;
        return true;
      }
      identifyState = 1;
    }
    default:
    break;
  }
  return false;
}


void verifyModelByGTIN(sDaliDriverData_t *sStaticData)
{
  if(0 == memcmp(sStaticData->sMemBnk0.gtin,&uGTIN.sGtinTable.sDexal.OTi30DX,sizeof(sGTIN_t)))
  {
      sStaticData->sStaticData.eDaliType    = evD4i;
      sStaticData->sStaticData.ratedWattage =    30;
//    sStaticData->sStaticData.eDaliType    = evDexal;
//    sStaticData->sStaticData.ratedWattage = 30;
//    sStaticData->sStaticData.fPowerUnit   = getDexalPowerUnitFloat();//power unit is static
//    sStaticData->sStaticData.fEnergyUnit  = getDexalEnergyUnitFloat();//energy unit is static
//    sStaticData->sStaticData.fRSTEnergyUnit = getDexalEnergyUnitFloat();
  }
  else if(0 == memcmp(sStaticData->sMemBnk0.gtin,&uGTIN.sGtinTable.sDexal.OTi50DX,sizeof(sGTIN_t)))
  {
    sStaticData->sStaticData.eDaliType    = evDexal;
    sStaticData->sStaticData.ratedWattage = 50;
    sStaticData->sStaticData.fPowerUnit   = getDexalPowerUnitFloat();//power unit is static
    sStaticData->sStaticData.fEnergyUnit  = getDexalEnergyUnitFloat();//energy unit is static
    sStaticData->sStaticData.fRSTEnergyUnit = getDexalEnergyUnitFloat();
  }
  else if(0 == memcmp(sStaticData->sMemBnk0.gtin,&uGTIN.sGtinTable.sDexal.OTi85DX,sizeof(sGTIN_t)))
  {
    sStaticData->sStaticData.eDaliType    = evDexal;
    sStaticData->sStaticData.ratedWattage = 85     ;
    sStaticData->sStaticData.fPowerUnit   = getDexalPowerUnitFloat();//power unit is static
    sStaticData->sStaticData.fEnergyUnit  = getDexalEnergyUnitFloat();//energy unit is static
    sStaticData->sStaticData.fRSTEnergyUnit = getDexalEnergyUnitFloat();
  }
  else if(0 == memcmp(sStaticData->sMemBnk0.gtin,&uGTIN.sGtinTable.sSR.XI040C110V054VPT1,sizeof(sGTIN_t)))
  {
    sStaticData->sStaticData.eDaliType    = evSR;
    sStaticData->sStaticData.ratedWattage = 40;
  }
  else if(0 == memcmp(sStaticData->sMemBnk0.gtin,&uGTIN.sGtinTable.sSR.XI040C110V054VPT2,sizeof(sGTIN_t)))
  {//This model is also d4i-capable, can be used to test d4i functionality
    sStaticData->sStaticData.eDaliType    = evSR;
//    sStaticData->sStaticData.eDaliType    = evD4i;
    sStaticData->sStaticData.ratedWattage =   40;
  }
  else if(0 == memcmp(sStaticData->sMemBnk0.gtin,&uGTIN.sGtinTable.sSR.XI075C200V054VPT1,sizeof(sGTIN_t)))
  {
    sStaticData->sStaticData.eDaliType     = evSR;
    sStaticData->sStaticData.ratedWattage  = 75;
  }//TODO: Add D4I Identification?
  else
  {
    sStaticData->sStaticData.eDaliType = evDali;//evDali;//TODO: set back to evDALI and actually test for D4i
  }
}