/**
 * @file dali.c
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <string.h>
#ifdef NRF
#include <zephyr.h>
#include <nrfx.h>
#include <nrfx_spim.h>
//#include "nrf_drv_spi.h"
//#include "nrf_log.h"
#else

#endif
#include "dali.h"
#include "dali_identify.h"
#include "dali_addressing.h"
#include "dali_staticData.h"
//#include "dali_dexal.h"
//#include "dali_sr.h"
#include "dali_d4i.h"
#include "dali_staticData.h"
#include "dali_power.h"
#include "dali_temperature.h"
#include "dali_LED_Load.h"
#include "dali_driver.h"
#include "dali_sequences.h"

typedef struct
{
    uint32_t pwr;
    float    fPwr;
    uint64_t nrg;
}sPwr_t;

typedef struct
{
    uint16_t volts;
    uint16_t  amps;
}sLEDLoadMeasurements_t;

typedef struct
{
    uint16_t temp;
}sGearMeasurements_t;

sPwr_t                 sPwr[5];//support 4 drivers.  The 5th element will take pwr readings for any index after the 4th.
sLEDLoadMeasurements_t sLEDLoadMsmts[5];
sGearMeasurements_t    sGearMsmts   [5];


/** @brief */
saDaliNetworkData_t saDaliNetworkData  __attribute__((aligned(4))) = 
{
    .numDrivers = 1,
    .uData = {0}
};

/** @brief  */
eDaliTaskStatus_t   eDaliTaskStatus = evDaliNoTaskRunning;

/** @brief */
sDaliTask_t sCurDaliTask =
{//currently executing dali task
  .eDaliTask      = evNoTask,
  .uTask.taskData = {0}
};

sDaliTask_t sIDaliTask = 
{//holder for interrupted task if higher priority (dimming) task is injected
    .eDaliTask = evNoTask,
    .uTask.taskData = {0}
};

_Bool    daliDataInitStatus = false;/*!<Inidcates if saDaliNetworkData has been populated or not*/
_Bool    bTaskValid         = false;/*!<Indicates internally if task is still being worked on*/  
_Bool    bDALITaskSuspended = false;/*!<Indicates a multi-transfer task was suspended for a higher priority dimming event*/

#ifdef NRF
 typedef struct k_timer daliTimer;
#else
typedef struct uint32_t daliTimer;//@TODO: fix!!!
#endif

#
void dali_periodic_fnc(daliTimer *pDaliTimer);

/**
 * @brief calculate checksum of memory region for verification of saDaliNetworkData in particular
 * 
 * @param pCKSMSrc start of memory to perform checksum over
 * @param CKSMLen number of bytes to take checksum of
 * @return uint8_t checksum
 */
uint8_t  Calc_Checksum      (uint8_t *pCKSMSrc, 
                             uint32_t CKSMLen );


void initDALI(void)
{
    daliInit();//sets up the spi interface, spi dma end of transfer interrupt, starts a spi transfer to force the line high
//    k_timer_init (&daliTimer, dali_periodic_fnc, NULL       );//init zephyr timer for DALI scheduling 
//    k_timer_start(&daliTimer, K_MSEC(25)      , K_MSEC(25));//start zephyr timer for DAIL scheduling
}

/**
 * @brief Assign DALI task if idle, return false if no idle, inject priority dimming tasks
 * @param psDaliTask Task to assign
 * @return False if not idle, true if task assigned
 */
_Bool setDaliTask(sDaliTask_t *psDaliTask)
{
#if 0//commented out to test injecting high priority dimming tasks code
    if(false == getDaliTransferStatus())
    {//return and indicate task not scheduled if DALI is mid-transfer
        return false;
    }
    if(eDaliTaskStatus != evDaliNoTaskRunning)
    {//return and indicate task not scheduled if DALI is in the middle of a multi-transfer task
        return false;
    }
    memcpy(&sCurDaliTask,psDaliTask,sizeof(sDaliTask_t));
    memset(psDaliTask,0,sizeof(sDaliTask_t));
    bTaskValid = true;
    return true;
#endif
    if(  (psDaliTask->eDaliTask  == evDaliSetLevel)  //allow task interruption if new task is dimming
       &&(sCurDaliTask.eDaliTask != evDaliSetLevel))//and a dimming task is not already scheduled (this shouldn't happen)
    {
        printk("Running task interrupted for high priority dimming task\n");
        memcpy(&sIDaliTask,&sCurDaliTask,sizeof(sDaliTask_t));//copy the interrupted task to be restored later
        bDALITaskSuspended = true;
    }
    else if(false == getDaliTransferStatus())
    {//return and indicate task not scheduled if DALI is mid-transfer
        return false;
    }
    else if(eDaliTaskStatus != evDaliNoTaskRunning)
    {//return and indicate task not scheduled if DALI is in the middle of a multi-transfer task
        return false;
    }
    memcpy(&sCurDaliTask,psDaliTask,sizeof(sDaliTask_t));
    memset(psDaliTask,0,sizeof(sDaliTask_t));
    bTaskValid = true;
    return true;
}


void dali_periodic_fnc(daliTimer *pDaliTimer)
{
    static uint8_t state = 0;
    static sDaliTask_t sDaliTask;
    
    eDaliTaskStatus_t eDaliTaskStatus;
    eDaliTaskStatus = daliManageTask();
}

//eDaliTaskStatus_t daliManageTask(sDaliTask_t *psDaliTask)
eDaliTaskStatus_t daliManageTask(void)
{
    if(false == getDaliTransferStatus())
    {//exit if transfers still in progress...this is critical
        return evDaliTaskRunning;
    }
    if(bTaskValid == false)
    {
      eDaliTaskStatus = evDaliNoTaskRunning;
//      return eDaliTaskStatus;
    }
    else if(  (true               == bTaskValid     ) 
            &&(evDaliTaskComplete == eDaliTaskStatus))
    {//This is to allow returning task complete response for single forward frame events without causing the frame to be re-sent
     //Suggests need to redesign
      bTaskValid = false;
      return evDaliTaskComplete;
    }
    switch(sCurDaliTask.eDaliTask)
    {
        case evDaliAddress:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == daliAddressingAlgorithm(&saDaliNetworkData.numDrivers))
            {
                printk("daliManageTask:addressing complete.\n");
                eDaliTaskStatus        = evDaliTaskComplete ;
                sCurDaliTask.eDaliTask = evNoTask           ;
                bTaskValid             = false              ;
            }
        break;
        case evDaliIdentify:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == identifyDaliDrivers(&saDaliNetworkData))
            {
                printk("daliManageTask:identifying complete.\n");
                
                saDaliNetworkData.checksum = Calc_Checksum((uint8_t *)&saDaliNetworkData,
                                                           sizeof(sDaliNetworkPld_t)    );
                eDaliTaskStatus        = evDaliTaskComplete;
                sCurDaliTask.eDaliTask = evNoTask          ;
                bTaskValid             = false             ;
            }
        break;
        case evDaliSetLevel:
            printk("Sending DAPC level %d to address %d, address type %d.\n",
                   sCurDaliTask.uTask.sSetDAPC.level       ,
                    sCurDaliTask.uTask.sSetDAPC.sAddrType.addr,
                    sCurDaliTask.uTask.sSetDAPC.sAddrType.eAddrType);
            sendCmdDapc(sCurDaliTask.uTask.sSetDAPC.sAddrType.addr     ,
                        sCurDaliTask.uTask.sSetDAPC.sAddrType.eAddrType,
                        sCurDaliTask.uTask.sSetDAPC.level              );
//            bTaskValid             = false             ;
            eDaliTaskStatus        = evDaliTaskComplete;
            sCurDaliTask.eDaliTask = evNoTask          ;
        break;
        case evDaliGetPwr:
            eDaliTaskStatus = evDaliTaskRunning;
//            if(true == readDALIPowerFloat(&saDaliNetworkData.uData[sCurDaliTask.uTask.sGetPwr.addr].sData,
//                                         &sCurDaliTask.uTask.sGetPwr.pfPwr ))
            if(true == readDALIPower(&saDaliNetworkData.uData[sCurDaliTask.uTask.sGetPwr.addr].sData,
                                     &sPwr[sCurDaliTask.uTask.sGetPwr.addr].pwr))
            {               
                eDaliTaskStatus        = evDaliTaskComplete;
                sCurDaliTask.eDaliTask = evNoTask           ;
                bTaskValid             = false              ;
                printk("Raw power read: %d\n",sPwr[sCurDaliTask.uTask.sGetPwr.addr].pwr);
            }
        break;
        case evDaliGetTotNrg:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == readDALIEnergy(&saDaliNetworkData.uData[sCurDaliTask.uTask.sGetPwr.addr].sData,
                                      &sPwr[sCurDaliTask.uTask.sGetNrg.addr].nrg)) 
            {
//              memcpy(psDaliTask,&sCurDaliTask,sizeof(sDaliTask_t));
              printk("Energy read: %llu\n",sPwr[sCurDaliTask.uTask.sGetNrg.addr].nrg);
              eDaliTaskStatus        = evDaliTaskComplete;
              sCurDaliTask.eDaliTask = evNoTask           ;
              bTaskValid             = false              ; 
            }
        break;
        case evDaliGetOutputCurrent:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == readDALILEDCurrent(&saDaliNetworkData.uData[sCurDaliTask.uTask.sGetIout.addr].sData,
                                          &sLEDLoadMsmts[sCurDaliTask.uTask.sGetIout.addr].amps))
            {
                eDaliTaskStatus        = evDaliTaskComplete;
                sCurDaliTask.eDaliTask = evNoTask           ;
                bTaskValid             = false              ; 
                printk("Raw current read: %d\n",sLEDLoadMsmts[sCurDaliTask.uTask.sGetPwr.addr].amps);
            }
        break;
        case evDaliGetOutputVoltage:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == readDALILEDVoltage(&saDaliNetworkData.uData[sCurDaliTask.uTask.sGetVout.addr].sData,
                                          &sLEDLoadMsmts[sCurDaliTask.uTask.sGetVout.addr].volts))
            {
                eDaliTaskStatus        = evDaliTaskComplete;
                sCurDaliTask.eDaliTask = evNoTask           ;
                bTaskValid             = false              ; 
                printk("Raw voltage read: %d\n",sLEDLoadMsmts[sCurDaliTask.uTask.sGetPwr.addr].volts);
            }
        break;
        case evDaliGetDriverTemperature:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == readDALIDriverTemperature(&saDaliNetworkData.uData[sCurDaliTask.uTask.sGetGearTemp.addr].sData,
                                                 &sGearMsmts[sCurDaliTask.uTask.sGetGearTemp.addr].temp))
            {
                eDaliTaskStatus        = evDaliTaskComplete;
                sCurDaliTask.eDaliTask = evNoTask           ;
                bTaskValid             = false              ;
                printk("Raw temperature read: %d\n", sGearMsmts[sCurDaliTask.uTask.sGetGearTemp.addr].temp); 
            }
        break;
        case evDaliPollForControlGear:
          eDaliTaskStatus = evDaliTaskRunning;
          if(true == daliPollForControlGear())
          {
            eDaliTaskStatus = evDaliTaskComplete;
            sCurDaliTask.eDaliTask = evNoTask  ;
            bTaskValid = false;
          }
          break;
        case evDaliReadMemoryBank:
            eDaliTaskStatus = evDaliTaskRunning;
            if(true == daliReadMB(&sCurDaliTask.uTask.sDaliReadMB))
            {
              eDaliTaskStatus        = evDaliTaskComplete;
              sCurDaliTask.eDaliTask = evNoTask           ;
              bTaskValid             = false              ; 
            }
            break;
        case evJCPHCommission:
          eDaliTaskStatus = evDaliTaskRunning;
          if(true == daliJCPHCommission(sCurDaliTask.uTask.sCommission.addrToSet,
                                        sCurDaliTask.uTask.sCommission.tuneVal))
          {
            eDaliTaskStatus = evDaliNoTaskRunning;
            sCurDaliTask.eDaliTask = evNoTask    ;
          }
          break;
        case evNoTask:
 //           eDaliTaskStatus = evDaliNoTaskRunning;
        break;
        default:
            eDaliTaskStatus        = evDaliNoTaskRunning;
            sCurDaliTask.eDaliTask = evNoTask           ;
        break;
    }
    if(true == transmitForwardFrame())
    {
        return evDaliTaskRunning;
    }
    if(bDALITaskSuspended)
    {
        memcpy(&sCurDaliTask,&sIDaliTask,sizeof(sDaliTask_t));//copy the interrupted task to be restored later
        bDALITaskSuspended = false;
        printk("Previously running task has been restored.\n");
    }
    return eDaliTaskStatus;
}


_Bool initDaliStaticData(const void * psaDaliNetworkData)
{
  memcpy(&saDaliNetworkData, psaDaliNetworkData, sizeof(saDaliNetworkData_t));
  if(  (saDaliNetworkData.numDrivers != 0           )//if num is not zero
     &&(saDaliNetworkData.numDrivers != 0xff        )//and not in erased state
     &&(saDaliNetworkData.checksum   ==              //and stored checksum
        Calc_Checksum((uint8_t *)&saDaliNetworkData,  //is valid
                      sizeof(sDaliNetworkPld_t)    ))
     )
  {
    daliDataInitStatus = true;
    return true;
  }
  memset(&saDaliNetworkData,0,sizeof(saDaliNetworkData_t));
  return false;
}


uint8_t Calc_Checksum(uint8_t *pCKSMSrc, uint32_t CKSMLen)
{
    uint8_t Cksm    = 0;
    uint8_t CksmCtr = 0;
    for(;CksmCtr<CKSMLen;CksmCtr++)
    {
        Cksm+= *(pCKSMSrc+CksmCtr);
    }
    return Cksm;
}


void * getAddressOfDaliData(void)
{
  return &saDaliNetworkData;
}


float getEnergyUnit(uint8_t addr)
{
  return saDaliNetworkData.uData[addr].sData.sStaticData.fEnergyUnit;
}


float getPowerUnit(uint8_t addr)
{
  return saDaliNetworkData.uData[addr].sData.sStaticData.fPowerUnit;
}


/**
 * @brief get the enumerated value of the currently running task
 * @return the enum of the current task
 */
eDaliTaskType_t   getCurDaliTask   (void)
{
  return sCurDaliTask.eDaliTask;
}

/**
 * @brief Returns true if a DALI task is running
 * @return _Bool true if DALI task is running
 */
_Bool isDaliTaskRunning(void)
{
if(  (false           == getDaliTransferStatus())
   ||(eDaliTaskStatus != evDaliNoTaskRunning    ))
{
  return true;//task is running
}
  return false;//Task is not running
}


void getDaliFlavor(uint8_t addr, uint8_t * pDaliType)
{
  if(addr > 3)
  {
    memcpy(pDaliType, "none",sizeof("none"));
  }
  else
  {
    if(saDaliNetworkData.uData[addr].sData.sStaticData.eDaliType == evSR)
    {
      memcpy(pDaliType, "SR",sizeof("SR"));
    }
    else if(saDaliNetworkData.uData[addr].sData.sStaticData.eDaliType == evDexal)
    {
     memcpy(pDaliType, "Dexal",sizeof("Dexal"));;
    }
    else if(saDaliNetworkData.uData[addr].sData.sStaticData.eDaliType == evD4i)
    {
     memcpy(pDaliType, "D4i",sizeof("D4i"));
    }
    else if(saDaliNetworkData.uData[addr].sData.sStaticData.eDaliType == evDali)
    {
     memcpy(pDaliType, "DALI",sizeof("DALI"));
    }
    else
    {
     memcpy(pDaliType, "none",sizeof("none"));
    }
  }
}

uint32_t getRatedDaliWattage(uint8_t addr)
{
  if(addr > 3)
  {
    return 0;
  }
  return saDaliNetworkData.uData[addr].sData.sStaticData.ratedWattage;
}

//uint32_t getDALIPower0(void)
//{
//    return getDALIPower(0);
//}
uint32_t getDALIPower(uint8_t index)
{
    if(index > 4)
    {
        index = 4;
    }
    return sPwr[index].pwr;
}

float getDALIPowerFloat(uint8_t index)
{
    if(index > 4)
    {
        index = 4;
    }
    sPwr[index].fPwr = (float)sPwr[index].pwr*saDaliNetworkData.uData[index].sData.sStaticData.fPowerUnit;
    return sPwr[index].fPwr;
}


uint32_t getDALIEnergy(uint8_t index)
{
    if(index > 4)
    {
        index = 4;
    }
    return (uint32_t)(sPwr[index].nrg);
}
uint16_t getDALILEDLoadVoltage(uint8_t index)
{
    if(index > 4)
    {
        index = 4;
    }
    return sLEDLoadMsmts[index].volts;
}

uint16_t getDALILEDLoadCurrent(uint8_t index)
{
    if(index > 4)
    {
        index = 4;
    }
    return sLEDLoadMsmts[index].amps;
}


uint16_t getDALiGearTemperature(uint8_t index)
{
    if(index > 4)
    {
        index = 4;
    }
    return sGearMsmts[index].temp;
}

//uint32_t getDALIEnergy