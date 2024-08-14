/**
 * @file dali_identify.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <stdbool.h>
#include "dali_staticData.h"

/**
 * @brief id DALI driver type by GTIN, get wattage rating and reporting units
 * 
 * @return _Bool 
 */
_Bool identifyDaliDrivers(saDaliNetworkData_t *);