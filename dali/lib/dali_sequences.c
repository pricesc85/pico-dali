//Source for dali multi-step sequences not covered by dali_addressing, memory banks, etc.
//These are not meant to be used with a dali task manager, as checking transmit status is
//handled within
#include <stdbool.h>
#include "dali_sequences.h"
#include "dali_commands.h"
#include "dali_driver.h"
#include "dali_frames.h"

uint8_t response = 0;
eRXDataStatus_t eRXDataStatus_l = evNoDataFound;


_Bool daliAssignAddress(uint8_t addr)
{
  static uint8_t state = 0;
  switch(state)
  {
  case 0://set dtr0 to new address
    sendSpecialCmdNoReply(addr, evSetDTR0);
//    transmitForwardFrame();
    state = 1;
  break;
  case 1://send set short address command
   if(true == getDaliTransferStatus())
   {
    sendStandardCmdTwice(0xff,evBroadcastAll,evSetShortAddressToDTR0);
//    transmitForwardFrame();
    state =2;
   }
  break;
  case 2:
  if(true == getDaliTransferStatus())
  {
    state = 0;
    return true;
  }
  break;
  }
  return false;
}

_Bool daliSingleAddressSequence(uint8_t addr)
{
  static uint8_t state = 0;

  switch(state)
  {
  case 0://query control gear present
  if(true == daliPollForControlGear())
  {
    state = 1;
  }
  break;
  case 1://clear address
  if(true == daliAssignAddress(0xff))
  {
    state = 2;
  }
  break;
  case 2://Assign new address
  if(true == daliAssignAddress((addr<<1)+1))
  {
    state = 0;
    return true;
  }
  break;
  }
  return false;
}

_Bool daliTuneULTDriver(uint8_t addr, uint8_t  tuneVal)
{
  uint8_t val = tuneVal;
  _Bool bDone = false;
  if(true == getDaliTransferStatus())
  {
  bDone = daliUnlockWriteLockMemoryBank(evShortAddress,//addrtype
                                       addr          ,//addr
                                       2             ,//membank#
                                       3             ,//index
                                       1             ,//numbytes
                                       &val          );//data ptr
                                       transmitForwardFrame();
  }
  return bDone;
}

_Bool daliJCPHCommission(uint8_t addr, uint8_t tuneVal)
{
  static uint8_t state = 0;
  switch(state)
  {
    case 0://driver found
      if(true == daliPollForControlGear())
      {
        state = 1;
      }
    break;
    case 2://Dim down for visual confirmation
    if(true == getDaliTransferStatus())
    {
      sendCmdDapc(addr,evShortAddress, 10);
      state = 3;
    }
    break;
    case 1://Adress it
      if(true == daliSingleAddressSequence(addr))
      {
        state = 2;
      }
    break;
    case 3://Tune it
      if(true == daliTuneULTDriver(addr,tuneVal))
      {
        state = 4;
      }
    break;
    case 4://Dim up using address for visual confirmation of assigned address and programmed tune value
    if(true == getDaliTransferStatus())
    {
      sendCmdDapc(addr,evShortAddress, 254);
      state = 5;
    }
    break;
    case 5://wait for DAPC to be done
    if(true == getDaliTransferStatus())
    {
      state = 0;
      return true;
    }
    break;
  }
  return false;
}

_Bool daliPollForControlGear(void)
{
  eRXDataStatus_t eRXDataStatus_l;
  static uint8_t state = 0;
  uint8_t pollResponse = 5;
  switch(state)
  {
    case 0:
      sendStandardCmdWithReply(0,evBroadcastAll, evQueryControlGearPresent);
      state = 1;
    break;
    case 1:
      state = 0;
      eRXDataStatus_l = getDaliBackFrame(&pollResponse);
      if(eRXDataStatus_l == evValidDataFound)
      { 
        if(pollResponse == 0xff)
        {
          return true;
        }
      }
    break;
  }
    return false;
}

