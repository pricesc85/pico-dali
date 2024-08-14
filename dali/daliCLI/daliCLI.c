#include "daliCLI.h"

#include "nrf52833.h"
#include "nrf52833_peripherals.h"
#include "nrf52833_bitfields.h"
#include "pca10100.h"
#include "log.h"

#include "..\dali.h"
#include "..\lib\dali_sequences.h"


//#define RX_PIN_NUMBER  6//22
//#define TX_PIN_NUMBER  21//20
//#define CTS_PIN_NUMBER 7
//#define RTS_PIN_NUMBER 5

uint8_t daliBuf[256] = {0};
/*DALI CLI protocol definitions*/
typedef struct
{
    union
    {
      uint8_t buffer[6];
      struct
      {
        uint8_t ctrl;
        union
        {
          struct
          {
            uint8_t addr;
            uint8_t bank;
            uint8_t index;
            uint8_t len;
          }sReadMemBank;
          struct
          {
            uint8_t addr;
            uint8_t bank;
            uint8_t index;
            uint8_t len;
            uint8_t val;
          }sWriteMemBank;
          struct
          {
            uint8_t addr;
            uint8_t level;
          }sSetDAPC;
          sCommission_t sCommission;
        };
      };
    };
}sDaliCLI_t;

#define READMEMBANK      1
#define WRITEMEMBANK     2
#define SETDAPC          3
#define COMMISSION       4
#define POLLFORCTRLGEAR  5
#define RETURNDALIDATA 254
#define ACK            255


sDaliTask_t sdTask = {0};
sDaliCLI_t sDaliCLI = {0};

uint8_t daliCLIBuffer[6] = {0};
volatile uint8_t dummyRead;

_Bool   uartErrCheck(void     );
_Bool   rxData      (uint8_t *);
_Bool   txData      (uint8_t  );


_Bool uartErrCheck(void)
{
  if(  (NRF_UART0->EVENTS_ERROR > 0)
     ||(NRF_UART0->ERRORSRC     > 0))
  {
    NRF_UARTE0->TASKS_FLUSHRX = 1;
    NRF_UART0->EVENTS_ERROR = 0x00000000;
    NRF_UART0->ERRORSRC     = 0x0000000F;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->TASKS_STARTTX = 1;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "error\n");
    
     return true;
  }
  return false;
}

_Bool rxData(uint8_t * pData)
{
  if(NRF_UART0->EVENTS_RXDRDY > 0)
  {
    NRF_UART0->EVENTS_RXDRDY = 0;
    *pData = NRF_UART0->RXD;
    return true;
  }
  return false;
}

_Bool txData(uint8_t data)
{
    if(NRF_UART0->EVENTS_TXDRDY == 1)
    {
      NRF_UART0->EVENTS_TXDRDY = 0;
    }
    NRF_UART0->TXD = data;
}


void daliCLIInit(void)
{
  NRF_UART0->ENABLE   = 4             ;//Turn it on
  NRF_UART0->PSEL.RTS = 5;//RTS_PIN_NUMBER; 
  NRF_UART0->PSEL.CTS =7;// CTS_PIN_NUMBER;
  NRF_UART0->PSEL.RXD = 6;//RX_PIN_NUMBER ;
  NRF_UART0->PSEL.TXD = 21;//TX_PIN_NUMBER ;
  NRF_UART0->BAUDRATE = 0x01D7E000    ;//115200 baud from product spec
//  NRF_UARTE0->CONFIG   = ();//default should be fine
  NRF_P0->PIN_CNF[6] = 12;//Connect, enable pullup
  NRF_P0->PIN_CNF[21] =14;//Enable pullup, leave input disonnected 
  NRF_UART0->TASKS_STARTRX = 1;
  NRF_UART0->TASKS_STARTTX = 1;
}

void daliCLIMinder(void)
{
  _Bool err;
  static uint8_t cliTxIterator     = 0;
  static uint8_t daliCLIcase       = 0;
  static uint8_t cliBufferIterator = 0;
  static uint8_t newAddress        = 0;
  static uint8_t newTuneVal        = 0;
//  txData(0x55);
//  return;
  eDaliTaskStatus_t eTaskStatus;
  eTaskStatus = daliManageTask();
  if(true == uartErrCheck())
  {
    daliCLIcase       = 0;
    cliBufferIterator = 0;
    return;
  }
  switch(daliCLIcase)
  {
  case 0://rcv
      while(true == rxData(&sDaliCLI.buffer[cliBufferIterator]))
      {
        cliBufferIterator++;
      }
      if(cliBufferIterator >= 6)
      {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "rcvd: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
          sDaliCLI.buffer[0],sDaliCLI.buffer[1],sDaliCLI.buffer[2],
          sDaliCLI.buffer[3],sDaliCLI.buffer[4],sDaliCLI.buffer[5]);
        daliCLIcase = sDaliCLI.buffer[0];
        switch(sDaliCLI.buffer[0])
        {
            case READMEMBANK:
                __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Read mem bank\n",
                sDaliCLI.buffer[0],sDaliCLI.buffer[1],sDaliCLI.buffer[2],sDaliCLI.buffer[3],sDaliCLI.buffer[4],sDaliCLI.buffer[5]);
                sdTask.eDaliTask                 = evDaliReadMemoryBank;
                if(sDaliCLI.sReadMemBank.addr > 63)
                {
                  sdTask.uTask.sDaliReadMB.eAddrType = evBroadcastAll;
                }
                sdTask.uTask.sDaliReadMB.addr    = sDaliCLI.sReadMemBank.addr;
                sdTask.uTask.sDaliReadMB.index   = sDaliCLI.sReadMemBank.index;
                sdTask.uTask.sDaliReadMB.memBank = sDaliCLI.sReadMemBank.bank;
                sdTask.uTask.sDaliReadMB.len     = sDaliCLI.sReadMemBank.len;
                sdTask.uTask.sDaliReadMB.cPtr    = daliBuf;
                setDaliTask(&sdTask);
                break;
            case WRITEMEMBANK:
                __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Write mem bank\n");
                if(sDaliCLI.sSetDAPC.addr > 63)
                {
                  sdTask.uTask.sDaliWriteMB.eAddrType = evBroadcastAll;
                }
                sdTask.eDaliTask = evDaliWriteMemoryBank;
//                sdTask.uTask.sDaliWriteMB.addr = sDaliCLI.u.sWriteTuneVal.addr;
                sdTask.uTask.sDaliWriteMB.index = 0x03; 
                sdTask.uTask.sDaliWriteMB.memBank = 0x02;
                sdTask.uTask.sDaliWriteMB.len = 0x01;
                memset(&daliBuf, 0, 256);
                daliBuf[0] = 53;
                sdTask.uTask.sDaliReadMB.cPtr = daliBuf;
                break;
            case SETDAPC://0x03 addr level xx xx xx  
              __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Set DAPC\n");
              sdTask.eDaliTask = evDAPC;
              sdTask.uTask.sSetDAPC.level = sDaliCLI.sSetDAPC.level;
              sdTask.uTask.sSetDAPC.sAddrType.addr = sDaliCLI.sSetDAPC.addr;
              if(sDaliCLI.sSetDAPC.addr > 63)
              {
                sdTask.uTask.sSetDAPC.sAddrType.eAddrType = evBroadcastAll;
              }
              else
              {
              sdTask.uTask.sSetDAPC.sAddrType.eAddrType = evShortAddress; 
              }
              setDaliTask(&sdTask);
              break;
            case COMMISSION://This is not managed by the task manager, so has to be handled differently
              __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Address and tune\n");
              sdTask.eDaliTask = evJCPHCommission;
              sdTask.uTask.sCommission.addrToSet = sDaliCLI.sCommission.addrToSet;
              sdTask.uTask.sCommission.tuneVal   = sDaliCLI.sCommission.tuneVal;
              setDaliTask(&sdTask);
            break;
            case POLLFORCTRLGEAR:
            sdTask.eDaliTask = evDaliPollForControlGear;
            setDaliTask(&sdTask);
            case ACK:
                break;
            default:
              sDaliCLI.buffer[0] = 0;
              sDaliCLI.buffer[1] = 0;
              sDaliCLI.buffer[2] = 0;
              sDaliCLI.buffer[3] = 0;
              sDaliCLI.buffer[4] = 0;
              sDaliCLI.buffer[5] = 0;
              cliBufferIterator = 0;
                break;
        }
      }
  break;
  case READMEMBANK:
    if(eTaskStatus != evDaliTaskRunning)
    {
      daliCLIcase = RETURNDALIDATA;
      cliBufferIterator = 0;
       __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "rcvd: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
       daliBuf[0],daliBuf[1],daliBuf[2],daliBuf[3],daliBuf[4],daliBuf[5],daliBuf[6],daliBuf[7],daliBuf[8],
       daliBuf[9],daliBuf[10],daliBuf[11],daliBuf[12],daliBuf[13],daliBuf[14],daliBuf[15])
    }
    break;
  case WRITEMEMBANK:
  
    break;
  case COMMISSION:
    if(eTaskStatus != evDaliTaskRunning)
    {
      daliCLIcase = 0;
      cliBufferIterator = 0;
      txData(0x55);
    }
    break;
  case SETDAPC:
      if(eTaskStatus != evDaliTaskRunning)
      {
        daliCLIcase       = 0;
        cliTxIterator     = 0;
        cliBufferIterator = 0;
      }
    break;
  case RETURNDALIDATA:
//    if(NRF_UART0->EVENTS_TXDRDY > 0)
//    {
      txData(daliBuf[cliTxIterator]);
      cliTxIterator++;
      if(cliTxIterator >= sDaliCLI.sReadMemBank.len)
      {
        cliTxIterator = 0;
        daliCLIcase       = 0;
        cliBufferIterator = 0;
      }
//    }
    break;
  case  ACK:
    txData(0x55);
    cliBufferIterator = 0;
    daliCLIcase = 0;
    break;
  default:
    cliBufferIterator = 0;
    daliCLIcase = 0;
  break;
  }
}


