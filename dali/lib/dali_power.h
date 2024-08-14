/**
 * @file dali_power.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "dali_staticData.h"
#include "dali.h"

/**
 * @brief Manages getting raw power readings from supported DALI flavors
 * 
 * @param psDaliDriverStaticData 
 * @param pPwr 32 bit raw power data, 4294967295 if feature not supported in selected driver
 * @return _Bool returns true when multi-byte read sequence is complete
 */
_Bool readDALIPower     (sDaliDriverData_t * psDaliDriverData, uint32_t * pPwr );

/**
 * @brief Manages getting raw Energy readings from supported DALI flavors
 * 
 * @param psDaliDriverStaticData 
 * @param pNrg 64 bit raw energy data, 0xFFFFFFFFFFFF if not supported in selected driver
 * @return _Bool returns true when multi-byte read sequence is complete
 */
_Bool readDALIEnergy    (sDaliDriverData_t * psDaliDriverData, uint64_t * pNrg );

/**
 * @brief Manages getting raw power data from supported DALI flavors and converting to float
 * 
 * @param psDaliDriverStaticData 
 * @param pfPwr Floating point power data, -1.0f if feature not supported in selected driver
 * @return _Bool returns true when multi-byte read sequence is complete
 */
_Bool readDALIPowerFloat(sDaliDriverData_t * psDaliDriverData, float    * pfPwr);
