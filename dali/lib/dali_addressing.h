/**
 * @file dali_addressing.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief Exposed function declarations for DALI addressing sequence
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdint.h>
#include <stdbool.h>


/**
 * @brief Addressing algorithm statemachine
 * 
 * @param numDrivers 
 * @return _Bool 
 */
_Bool daliAddressingAlgorithm(uint8_t * numDrivers);

/**
 * @brief Tells all drivers to initialize, resets addressing variables 
 * 
 * @return _Bool 
 */
_Bool daliResetAddressing    (void                );
