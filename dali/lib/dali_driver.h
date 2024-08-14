/**
 * @file dali_driver.h
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
#include "dali_frames.h"
#include "manchester.h"
#include "daliXfer.h"

/*configuration*/
#define DALI_SPI_INSTANCE  0 /**< SPI instance index. */
#define DALISPIINSTANCE spi0
#define DALI_TMR_INSTANCE  3 /**< Timer instance index.*/
#ifndef SPI_SCK_PIN
#define SPI_SCK_PIN 26
//#define SPI_SCK_PIN 4294967295
#endif

#ifndef SPI_MISO_PIN
#define SPI_MISO_PIN 29//30
#endif

#ifndef SPI_MOSI_PIN
#define SPI_MOSI_PIN 30//29
#endif

#ifndef SPI_SS_PIN
#define SPI_SS_PIN 31
#endif




/**
 * @brief Initialization of SPI interface and sends out a dummy forward frame to force the correct polarity on the pins 
 */
void daliInit(void);


/**
 * @brief Encodes and schedules for transmit a send once dali forward frame with no reply expected 
 * @param ufwdFrame Data to encode and transmit
 */
void transmitDaliCmdNoReply(uForwardFrame_t *ufwdFrame);


/**
 * @brief Encodes and schedules for transmit a send once dali forward frame with reply expected
 * @param ufwdFrame Data to encode and transmit
 */
void transmitDaliCmdWithReply(uForwardFrame_t *ufwdFrame);


/**
 * @brief Encodes and schedules for transmit a send twice dali forward frame with no reply expected
 * @param fwdFrame Data to encode and transmit
 */
void transmitDaliCmdTwice(uForwardFrame_t *ufwdFrame);


/**
 * @brief Get the status of current DALI transaction
 * @return _Bool Returns false if DALI transaction is underway, true if complete
 */
_Bool getDaliTransferStatus(void);


/**
 * @brief Begin the spi transmission of a forward frame scheduled by transmitDaliCmdTwice, transmitDaliCmdNoReply, or transmitDaliCmdWithReply
 * @return _Bool 
 */
_Bool transmitForwardFrame(void);


/**
 * @brief Get the backframe data from the SPI dma buffer, and decode
 * @param cptr decoded data is written here
 * @return eRXDataStatus_t 
 */
eRXDataStatus_t getDaliBackFrame (uint8_t *cptr);


