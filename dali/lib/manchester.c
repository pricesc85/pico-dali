/**
 * @file manchester.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <string.h>
#include "manchester.h"

#define RX_START_INDEX 38//first TE in which receive data may start

#define MANCHESTER_ZERO ((uint16_t)0x00ff)
#define MANCHESTER_ONE  ((uint16_t)0xff00)

#define MANCHESTER_IDLE ((uint16_t)0xffff)

uint8_t alignedbuf    [22]     =   {0 };
uint8_t alignedbufcopy[22]     =   {0 };


void manchesterEncodeBitTo16Bit(uint8_t bit, uint16_t *manchesterBit)
{
  if(1 == bit)
  {
    *manchesterBit = MANCHESTER_ONE;
  }
  else
  {
    *manchesterBit = MANCHESTER_ZERO;
  }
}


void manchesterEncodeMsg(uint8_t * msgtoEncode, uint8_t msgLength, uint8_t *manchesterBuffer)
{
  /*Encode start bit: logical one */
  manchesterEncodeBitTo16Bit(1,(uint16_t *)manchesterBuffer);
  manchesterBuffer += 2;
  /*Encode message*/
  while(msgLength != 0)
  {
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x80)>>7,(uint16_t *)(manchesterBuffer   ));//encode msb of byte
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x40)>>6,(uint16_t *)(manchesterBuffer+ 2));
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x20)>>5,(uint16_t *)(manchesterBuffer+ 4));
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x10)>>4,(uint16_t *)(manchesterBuffer+ 6));
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x08)>>3,(uint16_t *)(manchesterBuffer+ 8));
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x04)>>2,(uint16_t *)(manchesterBuffer+10));
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x02)>>1,(uint16_t *)(manchesterBuffer+12));
     manchesterEncodeBitTo16Bit(((*msgtoEncode)&0x01)>>0,(uint16_t *)(manchesterBuffer+14));//encode lsb of byte
     msgLength--;
     manchesterBuffer+=16;
     msgtoEncode     += 1;
  }
  /*Encode stop: 4 idle high TEs*/
  *(manchesterBuffer    ) = 0xff;
  *(manchesterBuffer + 1) = 0xff;
  *(manchesterBuffer + 2) = 0xff;
  *(manchesterBuffer + 3) = 0xff;
}


eRXDataStatus_t manchesterDecodeBackFrame(uint8_t *rxManBuf, uint8_t *decodedByte, uint8_t maxLen)
{
  uint8_t  decodeCtr        = 0x00;
  uint8_t  decodeIndex      = 0x00;
  uint8_t  shift            = 0x00;
  uint8_t  mask             = 0x00;
  uint8_t  setBitCount      = 0x00;
  const uint8_t  cBackFrameNumTEs =   22;      
  volatile eRXDataStatus_t eRXDataStatusReturn;
  while(decodeCtr++ < maxLen)
  {
    if(*(rxManBuf + decodeCtr) < 255)
    {//start bit found
      break;
    }
  }
  if(decodeCtr >= maxLen)
  {//no backward frame found
    eRXDataStatusReturn = evNoDataFound;
    return eRXDataStatusReturn;
  }
  else if((decodeCtr + 16) > maxLen)
  {//there appears to be data, but the buffer did not capture it
     eRXDataStatusReturn = evDataIncomplete;
    return eRXDataStatusReturn;
  }
 //find the bit index and masking into the first non-0xff byte for byte-aligning acquired data
  while(shift < 7)
  {
    if((*(rxManBuf+decodeCtr) & (0x80>>shift)) == 0)
    {//Stop once you find a zero.
      break;
    }
    mask += 1<<(7 - shift);
    shift++;
  }
  /*realign the data into another buffer*/
  while(decodeIndex < cBackFrameNumTEs)
  {/*Each iteration is 1 TE*/
    alignedbuf[decodeIndex] =   ((rxManBuf[decodeCtr +  decodeIndex    ]       )<<(shift    ))    
                              + ((rxManBuf[decodeCtr +  decodeIndex + 1] & mask)>>(8 - shift)); 
    /*convert bitfields into bit count      */
    decodeIndex++;
  }
  memcpy(&alignedbufcopy,&alignedbuf,sizeof(alignedbuf));
  decodeIndex = 0;
/*Add up the bits set per aligned byte*/
  while(decodeIndex < cBackFrameNumTEs)
  {
    while(alignedbuf[decodeIndex] != 0 )
    {
      setBitCount += (alignedbuf[decodeIndex] & 1);
      alignedbuf[decodeIndex] >>= 1;
    }

    /*convert bit count into 1 or 0 decision*/
    if(setBitCount < 4)
    {
      alignedbuf[decodeIndex] = 0;
    }
    else if((setBitCount == 4)&&(alignedbufcopy[decodeIndex] == 0xf0))
    {
      alignedbuf[decodeIndex] = 0;
    }
    else
    {
      alignedbuf[decodeIndex] = 1;
    }
    if(alignedbufcopy[decodeIndex] == 0x07)
    {
      alignedbuf[decodeIndex] = 1;
    }
    setBitCount = 0;
    decodeIndex++;
  }
  decodeIndex = 0;
 /*Check start bit validity*/
  if(  (alignedbuf[0] != 0)
     ||(alignedbuf[1] != 1))
  {//start bit pattern mismatch
   eRXDataStatusReturn = evDataCorrupt;
   return eRXDataStatusReturn;
  }
  /*Check stop bit validity*/
  if((alignedbuf[18] + alignedbuf[19] + alignedbuf[20] + alignedbuf[21]) < 3)
  {//stop bit pattern mismatch
  eRXDataStatusReturn = evDataCorrupt;
   return eRXDataStatusReturn;
  }
  /*decode TEs to received backward frame byte*/
  decodeIndex = 0;
  while(decodeIndex < 8)
  {
   if(  (alignedbuf[2 + (2*decodeIndex)] == 1)
      &&(alignedbuf[3 + (2*decodeIndex)] == 0))
   {
     *decodedByte &= ~(1<<(7 - decodeIndex));
   }
   else if(  (alignedbuf[2 + (2*decodeIndex)] == 0)
           &&(alignedbuf[3 + (2*decodeIndex)] == 1))
   {
     *decodedByte |= (1<<(7 - decodeIndex)); 
   }
   else
   {//data pattern mismatch (2 high or 2 low TEs)
    eRXDataStatusReturn = evDataCorrupt;
     return eRXDataStatusReturn;
   }
   decodeIndex += 1;
  }
  return evValidDataFound;
}




 