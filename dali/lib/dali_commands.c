/**
 * @file dali_commands.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief Functions for preparing DALI commands for transmission
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "dali_commands.h"
#include "dali.h"
#include "dali_frames.h"
#include "dali_driver.h"
#include "stdint.h"
#include "stdbool.h"

#define BROADCAST_UNADDRESSED 0xFC    /**<Forward frame address byte for broadcasting to unaddressed drivers, standard command*/
#define BROADCAST_ALL         0xFE    /**<Forward frame address byte for broadcasting to all drivers, standard command*/


#define MAX_SEARCH_ADDRESS    0xffffff/**< Search address is on the range [0:(2^24)-1]*/
#define MAX_SHORT_ADDRESS     63      /**< Short addresses take the range 0-63*/
#define MAX_GROUP_ADDRESS     15      /**<Group addresses take the range 0-15*/

uForwardFrame_t uForwardFrame;


void generateAddr(eDaliStandardAddressType_t eAddrType, uint8_t addr, uint8_t *dest)
{
    switch(eAddrType)
    {
        case evShortAddress:
            (*dest) = (addr <= MAX_SHORT_ADDRESS) ? (addr<<1) : (MAX_SHORT_ADDRESS<<1);
        break;
        case evGroupAddress:
            (*dest) = (addr <= MAX_GROUP_ADDRESS) ? (addr<<1) : (MAX_GROUP_ADDRESS<<1); 
            if(addr <= MAX_GROUP_ADDRESS)
        break;
        case evBroadcastUnaddressed:
            *dest = BROADCAST_UNADDRESSED;
        break;
        case evBroadcastAll:
            *dest = BROADCAST_ALL;
        break;
        default:
        break;
    }
}


void sendCmdDapc(uint8_t addr, eDaliStandardAddressType_t eAddrType, uint8_t lvl)
{//returns true if Dapc forward frame started.  returns false if something prevents it (bad address, bus busy)
  generateAddr(eAddrType, addr, &uForwardFrame.sStandardCmd.address);
  uForwardFrame.sStandardCmd.opcode = lvl;
  transmitDaliCmdNoReply(&uForwardFrame);
}

void sendSpecialCmdNoReply(uint8_t data, eDaliSpecialCommands_t eSpecialCmd)
{
    switch(eSpecialCmd)
    {
        /*No data commands*/
        case evTerminate:
        case evWithdraw: 
        case evPing: 
            data = 0;
        /*Data commands. Break intentionally omitted*/
        case evSetDTR0:
        case evSearchAddrH:
        case evSearchAddrM:
        case evSearchAddrL:
        case evprogramShortAddr:
        case evEnableDeviceType:
        case evSetDTR1:
        case evSetDTR2:
        case evWriteMemBnkNoReply:
          uForwardFrame.sSpecialCmd.data   = data;
          uForwardFrame.sSpecialCmd.opcode = (uint8_t)(eSpecialCmd);
          transmitDaliCmdNoReply(&uForwardFrame);
        break;
        default:
        break;
    }
}

void sendSpecialCmdTwice(uint8_t data, eDaliSpecialCommands_t eSpecialCmd)
{
  static uint8_t sendCmdState = 0;
  switch(eSpecialCmd)
  {
    case evRandomise :
      data = 0;
    case evInitialise:
            uForwardFrame.sSpecialCmd.data   = data;
            uForwardFrame.sSpecialCmd.opcode = (uint8_t)(eSpecialCmd);
            transmitDaliCmdTwice(&uForwardFrame);
            sendCmdState = 1;
    break;
    default:
    break;
  }
}


void sendStandardCmdWithReply(uint8_t addr, eDaliStandardAddressType_t eAddrType, eDaliStandardCommands_t eCmd)
{
    switch(eCmd)
    {
        case evQueryStatus:
        case evQueryControlGearPresent:
        case evQueryLampFailure:
        case evQueryLampPowerOn:
        case evQueryLimitError:
        case evQueryResetState:
        case evQueryMissingShortAddress:
        case evQueryVersionNumber:
        case evQueryContentDTR0:
        case evQueryDeviceType:
        case evQueryPhysicalMinimum:
        case evQueryPowerFailure:
        case evQueryContentDTR1:
        case evQueryContentDTR2:
        case evQueryOperatingMode:
        case evQueryLightSourceType:
        case evQueryActualLevel:
        case evQueryMaxLevel:
        case evQueryMinLevel:
        case evQueryPowerOnLevel:
        case evQuerySystemFailureLevel:
        case evQueryFadeTimeAndRate:
        case evQueryMfgSpecificMode:
        case evQueryNextDeviceType:
        case evQueryExtendedFadeTime:
        case evQueryControlGearFailure:
        case evQuerySceneXLevel:
        case evQueryGroups0To7:
        case evQueryGroups8To15:
        case evQueryRandomAddrH:
        case evQueryRandomAddrM:
        case evQueryRandomAddrL:
        case evReadMemoryBank:
        case evQueryExtendedVersionNum:
          generateAddr(eAddrType,addr,&uForwardFrame.sStandardCmd.address);
          uForwardFrame.sStandardCmd.address |= 1;
          uForwardFrame.sStandardCmd.opcode = (uint8_t)eCmd;
          transmitDaliCmdWithReply(&uForwardFrame);
        break;
        default:
        break;
     }
}


void sendSpecialCmdWithReply(uint8_t data                        ,
                             eDaliSpecialCommands_t eSpecialCmd  )
{
  switch(eSpecialCmd)
  {
    case evCompare:
    case evQueryShortAddr:
      data = 0;
    case evVerifyShortAddr:
    case evWriteMemoryBank:
      uForwardFrame.sSpecialCmd.opcode = (uint8_t)eSpecialCmd;
      uForwardFrame.sSpecialCmd.data   = data                ;
      transmitDaliCmdWithReply(&uForwardFrame);
    break;
    default:
    break;
  }
}                        

void  sendStandardCmdTwice(uint8_t addr                        ,
                           eDaliStandardAddressType_t eAddrType,
                           eDaliStandardCommands_t eStandardCmd)
{
  switch(eStandardCmd)
  {
  case evReset:
  case evStoreActualLevelInDTR0:
  case evSavePersistentVariables:
  case evSetOperatingModeToDTR0:
  case evResetMemoryBankDTR0:
  case evIdentifyDevice:
  case evSetMaxLevelToDTR0:
  case evSetMinLevelToDTR0:
  case evSetSystemFailureLevelToDTR0:
  case evSetPowerOnLevelToDTR0:
  case evSetFadeTimeToDTR0:
  case evSetFadeRateToDTR0:
  case evSetExtendedFadeTimeToDTR0:
  case evSetSceneXToDTR0:
  case evRemoveFromSceneX:
  case evAddToGroupX:
  case evRemoveFromGroupX:
  case evSetShortAddressToDTR0:
  case evEnableWriteMemory:
    generateAddr(eAddrType, addr, &uForwardFrame.sStandardCmd.address);
    uForwardFrame.sStandardCmd.address |= 1;
    uForwardFrame.sStandardCmd.opcode = (uint8_t)eStandardCmd;
    transmitDaliCmdTwice(&uForwardFrame);
  break;
  default:
  break;
}
}