/**
 * @file dali_dexal.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief Exposed function declarations for DEXAL-specific DALI sequences
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "dali.h"


/**
 * @brief Fetch power data from DEXAL driver and convert to float
 * 
 * @param addr 
 * @return _Bool Returns true when transaction complete
 */
_Bool getDexalPowerFloat         (uint8_t addr, float, float * );

/**
 * @brief Fetch raw power data from DEXAL driver
 * 
 * @param addr 
 * @return _Bool Returns true when transaction complete
 */
_Bool getDexalPowerRaw           (uint8_t addr, uint32_t * );

/**
 * @brief Fetch run time from DEXAL driver
 * 
 * @param addr 
 * @return _Bool Returns true when transaction complete
 */
_Bool getDexalLampRunTime        (uint8_t addr, uint32_t * );

/**
 * @brief Get the Dexal Max Case Temp
 * 
 * @param addr 
 * @return _Bool Returns true when transaction complete
 */
_Bool getDexalMaxCaseTemp        (uint8_t addr, uint8_t  * );

/**
 * @brief Get the Dexal Current Case Temp
 * 
 * @param addr 
 * @return _Bool Returns true when transaction complete
 */
_Bool getDexalCurrentCaseTemp    (uint8_t addr, uint8_t  * );

/**
 * @brief Get the Dexal Energy Raw energy data
 * 
 * @param addr 
 * @return _Bool Returns true when transaction complete
 */
_Bool getDexalEnergyRaw          (uint8_t addr, uint64_t * );

/**
 * @brief Get the Dexal Power Unit, convert to float
 * 
 * @return float 
 */
float getDexalPowerUnitFloat     (void                     );

/**
 * @brief Get the Dexal Energy Unit, convert to float
 * 
 * @return float 
 */
float getDexalEnergyUnitFloat    (void                     );



