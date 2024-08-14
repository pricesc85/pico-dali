/**
 * @file manchester.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief Interface to encoding and decoding DALI manchester messages from the SPI buffer
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <stdint.h>

/**
 * @brief status reception indicator enum
 * 
 */
typedef enum eRXDataStatus_t
{
  evValidDataFound,//!< Valid data was found in the raw data buffer
  evNoDataFound   ,//!< No data was found in the raw data buffer
  evDataCorrupt   ,//!< Received data is determined to be corrupt (useful for addressing)
  evDataIncomplete //!< Received data determined to be incomplete   
}eRXDataStatus_t;


/**
 * @brief Encodes individual manchester bits (2 TEs) to a 16 bit value
 * 
 * @param bit Bit (1 or 0) to encode to 16 bit value
 * @param manchesterBit 16 bit encoded manchester bit
 */
void            manchesterEncodeBitTo16Bit(uint8_t  bit             , //bit to encode
                                           uint16_t *manchesterBit  );//pointer to ui16 to store encoded bit  

/**
 * @brief Encodes a variable number of bytes to 16-bit encoded manchester message 
 * 
 * @param msgtoEncode Message to be encoded 
 * @param msgLength Length of message to encode, in number of bytes
 * @param manchesterBuffer buffer for 16-bit encoded Manchester data, prepared for sedning out spi port
 */
void            manchesterEncodeMsg       (uint8_t * msgtoEncode    , //pointer to byte array to encode 
                                           uint8_t length           , //number of bytes to encode
                                           uint8_t *manchesterBuffer);//pointer to manchester encoded buffer

/**
 * @brief searches the raw SPI buffer data for start of response (buffer contains tx and rx data)
 * 
 * @param rxManBuf Raw data to decode
 * @param decodedByte Decoded data
 * @param maxLen maximum number of bytes to search through in the raw data buffer
 * @return eRXDataStatus_t Decode status(data found, data not found, data incomplete, data corrupt)
 */
eRXDataStatus_t manchesterDecodeBackFrame (uint8_t *rxManchesterBuf , //pointer to raw spi received data
                                           uint8_t *backFrame       , //pointer to backwards frame buffer
                                           uint8_t charLength       );//number of bytes allocated to rxManchesterBuf to search through                                     
