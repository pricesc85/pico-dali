/**
 * @file dali_MemoryBank.h
 * @author Scott Price
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include "dali_commands.h"

typedef struct
{
    eDaliStandardAddressType_t eAddrType;
    uint8_t addr;
    uint8_t memBank;
    uint8_t index;
    uint8_t len;
    uint8_t * cPtr;
    
}sDaliReadMB_t;


/**
 * @brief Read bytes from a memory bank
 * 
 * @param addr Driver address to read
 * @param memoryBankNum memory bank to read
 * @param memoryBankStartIndex starting index into memory bank
 * @param numBytestoRead num bytes to read
 * @param cptr store read data here
 * @return _Bool 
 */
_Bool daliReadMemoryBank(eDaliStandardAddressType_t eAddrType,
                         uint8_t addr                        ,
                         uint8_t memoryBankNum               ,
                         uint8_t memoryBankStartIndex        ,
                         uint8_t numBytestoRead              ,
                         uint8_t *cptr                       );
/**
 * @brief Read bytes from mem bank, with pointer to setup instead of individual params
 * @return
 */
_Bool daliReadMB(sDaliReadMB_t *);

/**
 * @brief Write data to memory bank
 * 
 * @param numBytes 
 * @param psrc 
 * @return _Bool 
 */
_Bool daliWriteMemoryBank(uint8_t numBytes,
                          uint8_t *psrc   );

/**
 * @brief Unlock memory bank, write it, then lock it back
 * 
 * @param addr Driver address to write
 * @param memoryBankNum memory bank to write
 * @param index starting index into memory bank
 * @param numBytes number bytes to write
 * @param psrc data to write
 * @return _Bool 
 */
_Bool daliUnlockWriteLockMemoryBank (eDaliStandardAddressType_t eAddrType,
                                     uint8_t addr                        ,
                                     uint8_t memoryBankNum               ,
                                     uint8_t index                       ,
                                     uint8_t numBytes                    ,
                                     uint8_t *psrc                       );


