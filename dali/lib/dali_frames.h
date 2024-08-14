/**
 * @file dali_frames.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include <stdint.h>

#define NUM_BYTES_PER_TE            ( 1)
#define NUM_TES_PER_BIT             ( 2)
#define NUM_START_BITS              ( 1)
#define NUM_STOP_BITS               ( 2)
#define NUM_DATA_BITS_FORWARD_FRAME (16)
#define NUM_DATA_BITS_BACK_FRAME    ( 8)

#define SIZE_FORWARD_FRAME  ((NUM_DATA_BITS_FORWARD_FRAME + NUM_START_BITS + NUM_STOP_BITS)*NUM_TES_PER_BIT*NUM_BYTES_PER_TE)
#define SIZE_BACKWARD_FRAME ((NUM_DATA_BITS_BACK_FRAME    + NUM_START_BITS + NUM_STOP_BITS)*NUM_TES_PER_BIT*NUM_BYTES_PER_TE)

#define SEND_TWICE_FORWARD_FRAME_IDLE_TES (48)/*!< (20 milliseconds/417uS)*/ 
#define INTERFRAMEIDLE                    (36)/*!< 20 milliseconds,min time between fwd frames, can go as low as 13.5*/
#define MAXBYTESTOBACKFRAMESTART          (26)/*!< 10.5 milliseconds/417uS, round up*/



/**
 * @brief Union of decoded forward frame formats
 * 
 */
typedef union uForwardFrame_t
{
    uint8_t  ui8ForwardFrame [2];
    uint16_t ui16ForwardFrame   ;
    struct
    {
      uint8_t address;
      uint8_t opcode ;
    }sStandardCmd;
    struct
    {
      uint8_t opcode;
      uint8_t data  ;
    }sSpecialCmd;
}uForwardFrame_t;

/**
 * @brief Data structure for encoded Forward frame
 * 
 */
typedef struct sEncodedFwdFrame_t
{
  uint8_t encodedData[SIZE_FORWARD_FRAME];
}sEncodedFwdFrame_t;

/**
 * @brief Union of encoded single forward frame with send-twice forward frame, for transmit
 * 
 */
typedef union uEncodedFwdFrameBuf_t
{
  sEncodedFwdFrame_t sEncodedFwdFrame;
  struct
  {
    sEncodedFwdFrame_t sEncodedFwdFrame1       ;/*!< First forward frame in send twice */
    uint8_t            idleTime[INTERFRAMEIDLE];/*!< Idle time in send twice */
    sEncodedFwdFrame_t sEncodedFwdFrame2       ;/*!< Repeated forward frame in send twice */
    uint8_t            idleTime2[INTERFRAMEIDLE];/*!< Idle time before another frame can be sent*/
  }s2xFwdFrame;
}uEncodedFwdFrameBuf_t;


/**
 * @brief Union of DALI tx/rx transactions (rx sees tx), for rx buffer
 * 
 */
typedef union uRawDaliRXBuffer_t
{
  struct
  {//no reply with padding for delays
    uint8_t fwdFrameRegion[sizeof(sEncodedFwdFrame_t) + 2];/*!< No reply frame with padding for delays */
  }sRXNoReply;/*!<Rx structure for command with no response*/
  struct 
  {//send twice with idle time and padding for delays
    uint8_t fwdFrameRegion1[sizeof(sEncodedFwdFrame_t) + 2];/*!< First forward frame in send twice, with padding */
    uint8_t idleRegion     [INTERFRAMEIDLE             - 4];/*!< Idle between forward frames in send twice*/
    uint8_t fwdFrameRegion2[sizeof(sEncodedFwdFrame_t) + 2];/*!< First forward frame in send twice, with padding */
  }sRXSendTwice;/*!<Rx structure for command with no response, send twice*/
  struct
  {
    uint8_t fwdFrameRegion[sizeof(sEncodedFwdFrame_t) + 2];/*!<forward frame region with padding*/
    uint8_t backFrameRegion[MAXBYTESTOBACKFRAMESTART + SIZE_BACKWARD_FRAME];/*!<backframe window*/
  }sRXWithReply;/*!<Rx structure for command with response*/
}uRawDaliRXBuffer_t;

