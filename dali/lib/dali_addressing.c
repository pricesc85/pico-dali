/**
 * @file dali_addressing.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief functions handling DALI addressing
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "dali_addressing.h"
#include "dali_commands.h"
#include "dali_driver.h"
#include "dali_frames.h"

uint32_t        searchAddr[3] = {0xffffff,0x000000,0x000000};
uint8_t         curShortAddr  = 0;
eRXDataStatus_t eRXDataStatus;
uint8_t         compareResponse = 0;

/**
 * @brief Sets the 24 bit search address to use in the current iteration of the addressing algorithm
 * 
 * @param searchAddress 
 * @return _Bool 
 */
_Bool daliSetSearchAddress(uint32_t searchAddress);



_Bool daliAddressingAlgorithm(uint8_t * numDrivers)
{
  static uint32_t deltaGuess;
  static uint8_t addressingState = 0;
  switch(addressingState)
  {
  case 0://Put drivers in initialise mode
    daliResetAddressing();
    addressingState = 1;
    break;
  case 1://Tell initialised drivers to randomise
    sendSpecialCmdTwice(0,evRandomise);
    addressingState = 2;
    break;
  case 2://set up next search address guess
    if(true == daliSetSearchAddress((uint32_t)(searchAddr[0])&0xffffff))
     {
      addressingState = 8;
     }
    break;
  case 8:
    searchAddr[2] = searchAddr[1];
    searchAddr[1] = searchAddr[0];
    deltaGuess = (searchAddr[2] > searchAddr[1])? (searchAddr[2] - searchAddr[1]):(searchAddr[1] - searchAddr[2]);  
    eRXDataStatus = getDaliBackFrame(&compareResponse);
    if(  (evValidDataFound == eRXDataStatus)
       ||(evDataCorrupt    == eRXDataStatus))
    {//reduce address guess
      if(deltaGuess <= 1)
      {//search address has been found.
        sendSpecialCmdNoReply((curShortAddr<<1)|1,evprogramShortAddr);
        addressingState = 5;
        break;
      }
      else
      {
        searchAddr[0] =  searchAddr[1] - (deltaGuess>>1) - (deltaGuess%2);
      }
    }
    else
    {//increase address guess
      if(0xffffff == searchAddr[0])
      {
        addressingState = 7;
        break;
      }
      searchAddr[0] =  searchAddr[1] + (deltaGuess>>1) + (deltaGuess%2);
      searchAddr[0] = (searchAddr[0] > 0xffffff) ? 0xffffff : searchAddr[0];//clamp
    }
    if(false == daliSetSearchAddress((uint32_t)(searchAddr[0])&0xffffff))
    {
      addressingState = 2;
    }
    else
    {
      addressingState = 8;
    }    
    break;
  case 5://verify short address
    sendSpecialCmdWithReply((curShortAddr<<1)|1,evVerifyShortAddr);
    addressingState = 9;      
    break;
  case 9:
    eRXDataStatus = getDaliBackFrame(&compareResponse);
    if(  (evValidDataFound == eRXDataStatus)
       ||(evDataCorrupt    == eRXDataStatus))
    {
      sendSpecialCmdNoReply(0,evWithdraw);
      //start addressing over, searching for another driver
      searchAddr[0]   = 0xffffff;
      searchAddr[1]   = 0       ;
      searchAddr[2]   = 0       ;
      addressingState = 2       ;
      curShortAddr++            ;
    }
    else
    {//What do?
    }
  break;
  case 7://terminate.  End identification process
    sendSpecialCmdNoReply(0,evTerminate);
    addressingState = 10;
    break;
  case 10:
    *numDrivers     = curShortAddr;//inform calling function how many driversd were addressed.
    addressingState = 0;
    return true;
  default:
    break;
  }
  return false;
}

_Bool daliSetSearchAddress(uint32_t searchAddress)
{
//  static uint32_t lastAddr = 0;
  static uint8_t setSearchAddressState = 0;
  static uint32_t curAddr = 0;
  switch(setSearchAddressState)
  {
  case 0://Set high address
    if((curAddr & 0xff0000) != (searchAddress & 0xff0000))
    {
      sendSpecialCmdNoReply((uint8_t)((searchAddress&0xff0000)>>16),evSearchAddrH);
      curAddr = (curAddr & 0x00ffff) | (searchAddress & 0xff0000);
      setSearchAddressState = 1;
      break;
      
    }
  setSearchAddressState = 1;
  case 1://Set mid address
    if((curAddr & 0x00ff00) != (searchAddress & 0x00ff00))
    {
      sendSpecialCmdNoReply((uint8_t)((searchAddress&0xff00)>>8),evSearchAddrM);
      curAddr = (curAddr & 0xff00ff) | (searchAddress & 0x00ff00);
      setSearchAddressState = 2;
      break;
    }
    setSearchAddressState = 2;
  case 2://Set low address
    if((curAddr & 0x0000ff) != (searchAddress & 0x0000ff))
    {
      sendSpecialCmdNoReply((uint8_t)(searchAddress & 0xff),evSearchAddrL);
      curAddr = (curAddr & 0xffff00) | (searchAddress & 0x0000ff);
      setSearchAddressState = 3;
      break;
    }
    setSearchAddressState = 3;
  case 3://compare
    sendSpecialCmdWithReply(0,evCompare);
    setSearchAddressState = 0;
    return true;
  default:
  break;
  }
return false;
}


_Bool daliResetAddressing(void)
{//TODO: choose between initialize all and initialize all unaddressed?
    sendSpecialCmdTwice(0,evInitialise);//0 address tells all drivers to initialise.  All control gear will participate.
    searchAddr[0]   = 0xffffff;//reset stuff to initial values so they can be rerun
    searchAddr[1]   = 0;
    searchAddr[2]   = 0;
    curShortAddr    = 0;
    return true;
}