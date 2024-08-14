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
#include "dali_driver.h"
#include "dali.h"
#include "manchester.h"
#ifdef NRF
#include <zephyr.h>
#include <nrfx_spim.h>
#include <nrfx_timer.h>
#endif
//#include "log.h"
//#include "dali/daliCLI/daliCLI.h"

//SPI STUFF START
static volatile bool spiXferDone          = true;  /**< Flag used to indicate that SPI instance completed the transfer. */
static volatile bool frameReadyToTransmit = false;/**< Flag used to indicate a forward frame has been prepared to transmit*/      
uint8_t rxLen, txLen;
uEncodedFwdFrameBuf_t      uEncodedFwdFrame;
uRawDaliRXBuffer_t         uRawDaliRXBuffer;

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

