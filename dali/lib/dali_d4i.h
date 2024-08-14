/**
 * @file dali_d4i.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief Exposed function declarations for D4I-specific DALI sequences
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "dali_staticData.h"


#define SIZE_MB_202  16
#define SIZE_MB_203  16
#define SIZE_MB_204  16
#define SIZE_MB_205  29
#define SIZE_MB_206  33
#define SIZE_MB_207   8
#define SIZE_MB_1   120

/**
 * @brief Fetches D4I energy units
 * 
 * @return _Bool 
 */
_Bool getD4iUnits    (sDaliDriverData_t *);

/**
 * @brief Fetches raw D4I power data
 * 
 * @param addr 
 * @param pRawPwr 
 * @return _Bool 
 */
_Bool getD4iPowerRaw (uint8_t addr       , 
                      uint32_t * pRawPwr );

/**
 * @brief Fetches raw D4I energy data (nonresettable)
 * 
 * @param addr 
 * @param pNrgRaw 
 * @return _Bool 
 */
_Bool getD4iEnergyRaw(uint8_t addr       ,
                      uint64_t * pNrgRaw );

/**
 * @brief Fetches raw D4I power data and converts to float
 * 
 * @param addr 
 * @param floatUnit 
 * @param pFloatPwr 
 * @return _Bool 
 */
_Bool getD4iPowerFloat(uint8_t addr      , 
                       float floatUnit   ,
                       float * pFloatPwr);


_Bool getD4iLightSrcVoltage(uint8_t addr, uint16_t *pVolts);

_Bool getD4iLightSrcCurrent(uint8_t addr, uint16_t *pAmps);

_Bool getD4iGearTemperature(uint8_t addr, uint16_t *pTemp);

_Bool getMB202(uint8_t addr, uint8_t * dataPtr);


_Bool getMB203(uint8_t addr, uint8_t * dataPtr);


_Bool getMB204(uint8_t addr, uint8_t *dataPtr);


_Bool getMB205(uint8_t addr, uint8_t *dataPtr);


_Bool getMB206(uint8_t addr, uint8_t *dataPtr);


_Bool getMB207(uint8_t addr, uint8_t *dataPtr);


_Bool getMB1(uint8_t addr, uint8_t *dataPtr);

_Bool getD4iMemBanks(uint8_t addr, uint8_t *dataPtr);

