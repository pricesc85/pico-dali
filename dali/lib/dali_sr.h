/**
 * @file dali_sr.h
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
#include <stdbool.h>
#include "dali_staticData.h"


/**
 * @brief Unlocks the password protected SR readouts, such as power and energy
 * 
 * @param address 
 * @return _Bool Returns true when complete
 */
_Bool srUnlockPowerReading(uint8_t address    );

/**
 * @brief Gets the raw power data
 * 
 * @param address 
 * @param pwr 
 * @return _Bool Returns true when complete
 */
_Bool getSRPowerRaw       (uint8_t address    , 
                           uint32_t *pwr      );

/**
 * @brief Gets the raw power data and converts to float
 * 
 * @param address 
 * @param pfPwr 
 * @return _Bool Returns true when complete
 */
_Bool getSRPowerFloat     (uint8_t address    , 
                           float              ,
                           float *pfPwr       );

/**
 * @brief Gets the raw energy data
 * 
 * @param address 
 * @param nrg 
 * @return _Bool Returns true when complete
 */
_Bool getSREnergyRaw      (uint8_t address    ,
                           uint64_t *nrg      );

/**
 * @brief Get the unit of measurement for power, energy, etc.
 * 
 * @return _Bool Returns true when complete
 */
_Bool getSRUnits          (sDaliDriverData_t *);

/**
 * @brief returns lock status of SR driver at address
 * 
 * @param address 
 * @return _Bool true if locked, false if unlocked or address out of bounds
 */
_Bool getSRLockStatus     (uint8_t address    );