/**
 * @file dali_driver.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "dali_driver.h"
#include "dali.h"
#include "manchester.h"
#ifdef NRF
#include <zephyr.h>
#include <nrfx_spim.h>
#include <nrfx_timer.h>
#else
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#endif
//#include "log.h"
//#include "dali/daliCLI/daliCLI.h"

//SPI STUFF START
static volatile bool spiXferDone          = true;  /**< Flag used to indicate that SPI instance completed the transfer. */
static volatile bool frameReadyToTransmit = false;/**< Flag used to indicate a forward frame has been prepared to transmit*/      
uint8_t rxLen, txLen;
uEncodedFwdFrameBuf_t      uEncodedFwdFrame;
uRawDaliRXBuffer_t         uRawDaliRXBuffer;



//int dma_chan;
uint dma_tx;
uint dma_rx; 


//void initializeDALI(void);

#ifdef NRF
#define SPIM_NODE DT_NODELABEL(DALISPIINSTANCE)
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(DALI_SPI_INSTANCE);
nrfx_spim_xfer_desc_t spim_xfer_desc =
{
  .p_tx_buffer = &uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0] ,
  .tx_length   = sizeof(sEncodedFwdFrame_t),
  .p_rx_buffer = (uint8_t *)&uRawDaliRXBuffer,
  .rx_length   = sizeof(uRawDaliRXBuffer.sRXNoReply)
};
#else
//TODO: FIX!!!!
#endif

/**
 * @brief Called upon spi event interrupt, sets spiXferDone flag to indicate transaction complete
 * @param p_event 
 * @param p_context 
 */
#ifdef NRF
void spi_event_handler(nrfx_spim_evt_t const * p_event,
                       void *                p_context)
{
    spiXferDone = true;
}
#else
void spi_event_handler(void)
{
  dma_hw->ints0 = 1u << dma_rx;
  printf("SPI DONE\n");
  spiXferDone = true;
}
#endif
//SPI STUFF END

void daliInit(void)
{
    memset(&uEncodedFwdFrame,0xff,sizeof(uEncodedFwdFrame));
#ifdef NRF    
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG(SPI_SCK_PIN ,
                                                             SPI_MOSI_PIN,
                                                             SPI_MISO_PIN,
                                                             SPI_SS_PIN  );
    spi_config.frequency = (0x00500000UL);
    nrfx_spim_init(&spi,&spi_config, spi_event_handler, NULL);
    IRQ_CONNECT(DT_IRQN(SPIM_NODE          ),
                DT_IRQ (SPIM_NODE, priority),
                nrfx_isr, 
                nrfx_spim_0_irq_handler,
                0);
    irq_enable(DT_IRQN(SPIM_NODE));
    nrfx_spim_xfer(&spi,&spim_xfer_desc,0);
    
    NRF_P0->PIN_CNF[29] &= ~(3<<2);
//    k_timer_init (&daliTimer, dali_periodic_fnc, NULL       );//init zephyr timer for DALI scheduling 
//    k_timer_start(&daliTimer, K_MSEC(100)      , K_MSEC(100));//start zephyr timer for DAIL scheduling
#else
    spi_init(spi_default, 19200);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    
    dma_tx = dma_claim_unused_channel(true);
    dma_rx = dma_claim_unused_channel(true);

   // We set the outbound DMA to transfer from a memory buffer to the SPI transmit FIFO paced by the SPI TX FIFO DREQ
    // The default is for the read address to increment every element (in this case 1 byte = DMA_SIZE_8)
    // and for the write address to remain unchanged.

    printf("Configure TX DMA\n");
    dma_channel_config c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    dma_channel_configure(dma_tx, &c,
                          &spi_get_hw(spi_default)->dr                     , // write address
                          &uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0], // read address
                          sizeof(sEncodedFwdFrame_t)                       , // element count (each element is of size transfer_data_size)
                          false                                            ); // don't start yet

    printf("Configure RX DMA\n");

    // We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
    // We configure the read address to remain unchanged for each element, but the write
    // address to increment (so data is written throughout the buffer)
    c = dma_channel_get_default_config(dma_rx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    dma_channel_configure(dma_rx, &c,
                           (uint8_t *)&uRawDaliRXBuffer      , // write address
                          &spi_get_hw(spi_default)->dr       , // read address
                          sizeof(uRawDaliRXBuffer.sRXNoReply), // element count (each element is of size transfer_data_size)
                          false                              ); // don't start yet
  // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_rx, true);

    // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, spi_event_handler);
    irq_set_enabled(DMA_IRQ_0, true);
#endif

}



void transmitDaliCmdNoReply(uForwardFrame_t *ufwdFrame)
{
  memset(&uRawDaliRXBuffer                                    ,
         0x00                                                 ,
         sizeof(uRawDaliRXBuffer)                             );
  memset(&uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0]    ,
         0xff                                                 ,
         sizeof(uEncodedFwdFrame.sEncodedFwdFrame.encodedData));

  manchesterEncodeMsg((uint8_t *)ufwdFrame                             ,
                      2                                                ,
                      &uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0]);
  rxLen = sizeof(sEncodedFwdFrame_t) + INTERFRAMEIDLE;
  txLen = sizeof(sEncodedFwdFrame_t);
  frameReadyToTransmit = true;
  spiXferDone          = false;
}

void transmitDaliCmdWithReply(uForwardFrame_t *ufwdFrame)
{
  memset(&uRawDaliRXBuffer                    ,
         0x00                                 ,
         sizeof(uRawDaliRXBuffer.sRXWithReply));
  memset(&uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0]    ,
         0xff                                                 ,
         sizeof(uEncodedFwdFrame.sEncodedFwdFrame.encodedData));
  manchesterEncodeMsg((uint8_t *)ufwdFrame                             ,
                      2                                                ,
                      &uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0]);
  rxLen = sizeof(uRawDaliRXBuffer.sRXWithReply);
  txLen = sizeof(sEncodedFwdFrame_t);
  frameReadyToTransmit = true;
  spiXferDone          = false;
}


void transmitDaliCmdTwice(uForwardFrame_t *fwdFrame)
{
  memset(&uRawDaliRXBuffer                    ,
         0x00                                 ,
         sizeof(uRawDaliRXBuffer.sRXSendTwice));
  memset(&uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0]    ,
         0xff                                                 ,
         sizeof(uEncodedFwdFrame.sEncodedFwdFrame.encodedData));
  manchesterEncodeMsg((uint8_t *)fwdFrame,2,&uEncodedFwdFrame.s2xFwdFrame.sEncodedFwdFrame1.encodedData[0]);
  manchesterEncodeMsg((uint8_t *)fwdFrame,2,&uEncodedFwdFrame.s2xFwdFrame.sEncodedFwdFrame2.encodedData[0]);//repeat
  rxLen = sizeof(uRawDaliRXBuffer.sRXSendTwice);
  txLen = sizeof(uEncodedFwdFrame.s2xFwdFrame);
  frameReadyToTransmit = true;
  spiXferDone          = false;                                   
}

_Bool getDaliTransferStatus(void)
{
    return spiXferDone;
}


eRXDataStatus_t getDaliBackFrame(uint8_t *cptr)
{
 return manchesterDecodeBackFrame(&uRawDaliRXBuffer.sRXWithReply.backFrameRegion[0]    ,
                                  cptr                                                 ,
                                  sizeof(uRawDaliRXBuffer.sRXWithReply.backFrameRegion));
}


_Bool transmitForwardFrame(void)
{
  if(true == frameReadyToTransmit)
  {
    frameReadyToTransmit = false;
    #ifdef NRF
    spiXferDone = false;
    spim_xfer_desc.tx_length = txLen;
    spim_xfer_desc.rx_length = rxLen;
    nrfx_spim_xfer(&spi,&spim_xfer_desc,0) ;
    #else
    spiXferDone = false;
    printf("Configure TX DMA\n");
    dma_channel_config c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    dma_channel_configure(dma_tx, &c,
                          &spi_get_hw(spi_default)->dr                     , // write address
                          &uEncodedFwdFrame.sEncodedFwdFrame.encodedData[0], // read address
                          txLen                       , // element count (each element is of size transfer_data_size)
                          false                                            ); // don't start yet

    printf("Configure RX DMA\n");

    // We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
    // We configure the read address to remain unchanged for each element, but the write
    // address to increment (so data is written throughout the buffer)
    c = dma_channel_get_default_config(dma_rx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    dma_channel_configure(dma_rx, &c,
                           (uint8_t *)&uRawDaliRXBuffer      , // write address
                          &spi_get_hw(spi_default)->dr       , // read address
                          rxLen, // element count (each element is of size transfer_data_size)
                          false                              ); // don't start yet
    dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
    #endif
    return true;
  }
  return false;
}

#if 0
void timerDALIeventHandler(nrf_timer_event_t event_type, void *p_context)
{

  switch(event_type)
  {
    case NRF_TIMER_EVENT_COMPARE3:
#ifdef DALICLI
      daliCLIMinder();
#endif
    daliManageTask();
    break;
    default:
    break;
  }
}
#endif

