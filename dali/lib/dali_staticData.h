/**
 * @file dali_staticData.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <stdint.h>
//#include "dali.h"

#define SIZE_GTIN             6
#define SIZE_IDNUM            8

//#define MAX_SUPPORTED_DRIVERS 4

/**
 * @brief dali types enumeration
 * 
 */
typedef enum
{
evDali,
evDexal,
evD4i,
evSR
}eDaliType_t;

/**
 * @brief struct for version info stored in memory bank 0
 * 
 */
typedef union
{
  uint8_t version[2];
  struct
  {
    uint8_t majorVer;
    uint8_t minorVer;
  }sVer_t;
}uHwFwVersion_t;

/**
 * @brief struct for storing a driver's gtin
 * 
 */
typedef struct
{
  uint8_t gtin[SIZE_GTIN];
}sGTIN_t;


/**
 * @brief struct for holding data stored in memory bank 0
 * 
 */
typedef struct
{
  uint8_t        gtin [SIZE_GTIN]    ;
  uHwFwVersion_t uFwVersion          ;
  uint8_t        idNum[SIZE_IDNUM]   ;
  uHwFwVersion_t uHwVersion          ;
  uint8_t        verPt101            ;
  uint8_t        verPt102            ;
  uint8_t        verPt103            ;
  uint8_t        numLogicalCtrlDevice;
  uint8_t        numLogicalCtrlGear  ;
  uint8_t        curCtrlGearUnitIndex;
/*extra data not in memory banks*/
}sMemoryBank0_t;

/**
 * @brief struct for holding useful per driver information
 * 
 */
typedef struct
{
  eDaliType_t    eDaliType        ;
  uint8_t        addr             ;
  uint32_t       ratedWattage     ;
  float          fPowerUnit       ;
  float          fEnergyUnit      ;
  float          fRSTEnergyUnit   ;
  float          fRunTimeUnit     ;
}sStaticData_t;

/**
 * @brief struct for holding user configuration information (TBD, things like groups, scenes, max and min levels when implemented)
 * 
 */
typedef struct
{

}sUserConfig_t;


/**
 * @brief struct combining all the static-ish per driver data together
 * 
 */
typedef struct
{
  sMemoryBank0_t  sMemBnk0;
  sStaticData_t   sStaticData;
  sUserConfig_t   sUserConfig;
}sDaliDriverData_t;

/**
 * @brief union of sDaliDriverData_t with a same-sized data-array, for easy of memcpying
 * 
 */
typedef union
{
  sDaliDriverData_t sData;
  uint8_t           aStaticData[sizeof(sDaliDriverData_t)];
}uDaliDriverData_t;



