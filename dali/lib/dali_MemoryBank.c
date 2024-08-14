/**
 * @file dali_MemoryBank.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdint.h>
#include <stdbool.h>
//#include "nrf_log.h"
#include "dali_MemoryBank.h"
#include "dali_commands.h"
#include "dali_driver.h"
/*Local function prototypes*/

/**
 * @brief Sets data transfer registers to memory bank and offset for subsequent memory bank read
 * 
 * @param memoryBankNum 
 * @param offset 
 * @return _Bool 
 */
_Bool daliSetMemoryBankandOffset    (uint8_t memoryBankNum,
                                     uint8_t offset       );

/**
 * @brief Enable writing of memory location
 * 
 * @param addr 
 * @return _Bool 
 */
_Bool daliEnableWriteMemory         (eDaliStandardAddressType_t eAddr,
                                     uint8_t addr                    );


_Bool daliEnableWriteMemory(eDaliStandardAddressType_t eAddr,
                            uint8_t addr                    )
{
  sendStandardCmdTwice(addr,eAddr,evEnableWriteMemory);
  return true; 
}


_Bool daliSetMemoryBankandOffset(uint8_t memoryBankNum, uint8_t offset)
{
  static uint8_t setMemBankState = 0;
  switch(setMemBankState)
  {
  case 0:
    sendSpecialCmdNoReply(memoryBankNum,evSetDTR1);
    setMemBankState = 1;
  break;
  case 1:
    sendSpecialCmdNoReply(offset,evSetDTR0);
    setMemBankState = 0;
    return true;
  break;
  }
  return false;
}


_Bool daliReadMB(sDaliReadMB_t * sDaliReadMB)
{
    return daliReadMemoryBank(sDaliReadMB->eAddrType,
                              sDaliReadMB->addr     ,
                              sDaliReadMB->memBank  ,
                              sDaliReadMB->index    ,
                              sDaliReadMB->len      ,
                              sDaliReadMB->cPtr     );
}
_Bool daliReadMemoryBank(eDaliStandardAddressType_t eAddrType           ,
                         uint8_t                    addr                ,
                         uint8_t                    memoryBankNum       ,
                         uint8_t                    memoryBankStartIndex,
                         uint8_t                    numBytestoRead      ,
                         uint8_t                    *cptr               )
{
  static uint8_t readNum          = 0;
  static uint8_t readMemBankState = 0;
  switch(readMemBankState)
  {
  case 0:
    if(true == daliSetMemoryBankandOffset(memoryBankNum, memoryBankStartIndex))
    {
      readMemBankState = 1;
    }
  break;
  case 1://Read memory bank
    sendStandardCmdWithReply(addr, eAddrType,evReadMemoryBank);
    readMemBankState = 2;
  break;
  case 2://sp added 2/6/2020
    getDaliBackFrame(cptr + readNum);
    if(readNum >= (numBytestoRead-1))
    {
      readNum = 0;
      readMemBankState = 0;
      return true;
    }
    else
    {
      readNum++;
      sendStandardCmdWithReply(addr, eAddrType,evReadMemoryBank);
      //readMemBankState = 1;
    }
  default:
    break;
  }
  return false;
}


_Bool daliWriteMemoryBank(uint8_t numBytes, uint8_t *pSrc)
{
static uint8_t pCnt = 0;
  sendSpecialCmdWithReply(*(pSrc + pCnt),evWriteMemoryBank);
  pCnt++;
  if(pCnt >= numBytes)
  {
    pCnt = 0;
    return true;
  }
return false;
}



_Bool daliUnlockWriteLockMemoryBank(eDaliStandardAddressType_t eAddrType,
                                    uint8_t addr                        ,
                                    uint8_t memoryBankNum               ,
                                    uint8_t index                       ,
                                    uint8_t numBytes                    ,
                                    uint8_t *psrc                       )
{
  static uint8_t mbState = 0;
  switch(mbState)
  {
    case 0://EnableWriteMemory
      if(true == daliSetMemoryBankandOffset(memoryBankNum, 2))
      {
        mbState = 1;
      }
    break;
    case 1://Set DTRs to memory bank and index for lock byte
      daliEnableWriteMemory(eAddrType, addr);
      mbState = 2;
    break;
    case 2://write 0x55 to lock byte to unlock memory bank
      sendSpecialCmdWithReply(0x55, evWriteMemoryBank);//evWriteMemBnkNoReply);
      mbState = 3;
    break;
    case 3://Set DTR to index for write
      sendSpecialCmdNoReply(index,evSetDTR0);
      mbState = 4;
    break;
    case 4:
      daliEnableWriteMemory(eAddrType, addr);
      mbState = 5;
    break;
    case 5://perform write actions
    if(true == daliWriteMemoryBank(numBytes, psrc))
    {
      mbState = 6;
    }
    break;
    case 6://Set DTR back to index for lock byte
      sendSpecialCmdNoReply(2,evSetDTR0);
      mbState = 7;
    break;
    case 7:
      daliEnableWriteMemory(eAddrType, addr);
      mbState = 8;
    break;
    case 8://Write 0xff to lock byte to lock back
      sendSpecialCmdWithReply(0xff, evWriteMemoryBank);//evWriteMemBnkNoReply);
      mbState = 0;
    return true;
    break;
    default:
    break;
  }
  return false;
}
