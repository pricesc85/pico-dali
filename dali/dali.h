/**
 * @file dali.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief 
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include "stdint.h"
#include "dali_staticData.h"
#include "dali_commands.h"
#include "dali_maxDeviceSupport.h"
#include "dali_MemoryBank.h"

//#define DALICLI


/*! Enum of supported scheduleable DALI task types */
typedef enum
{
  evNoTask = 0   ,/*This should remain 0 so memset to 0 sets to evNoTask*/
  evDaliAddress  ,/*Run addressing algorithm                           */
  evDaliIdentify ,/*Identify drivers discovered by addressing algorithm. Will invoke addressing if DALI code does not have a list of drivers*/ 
  evDaliSetLevel ,/*Set DAPC level...set output current, 0-255*/
  evDaliGetPwr   ,/*Query power consumption from a driver*/
  evDaliGetTotNrg,/*Query total energy consumption from a driver*/
  evDaliGetOutputCurrent,/*Query LED load current*/
  evDaliGetOutputVoltage,/*Query LED load voltage*/
  evDaliGetDriverTemperature,/*Query driver temperature*/
  evDaliReadMemoryBank,
  evDaliWriteMemoryBank,
  evDaliPollForControlGear,
  evJCPHCommission/*For hospital retrofit, pre-address and tune ULT driver*/
}eDaliTaskType_t;


/*! Enum of possible DALI bus-related task status, multi-transaction tasks will keep at evDaliTaskRunning until the final transaction is complete*/
typedef enum
{
  evDaliNoTaskRunning,
  evDaliTaskRunning,
  evDaliTaskComplete,
  evDaliTaskNotSupported
}eDaliTaskStatus_t;

/*! Struct of address and address type to target*/
typedef struct
{
    uint8_t addr;
    eDaliStandardAddressType_t eAddrType;
}sAddrType_t;

/*! Struct of address and level for DAPC command*/
typedef struct 
{
    sAddrType_t sAddrType;
    uint8_t level;
}sDaliDAPC_t;

/*! struct of address to read power from, and float conversion of the returned power data*/
typedef struct 
{
    uint8_t addr;
    float   pfPwr;    
}sDaliPwr_t;

/*! struct of address to read energy from and returned energy data*/
typedef struct 
{
    uint8_t  addr;
    uint64_t pNrg;
}sDaliNrg_t;

typedef struct
{
    uint8_t addr;
    uint16_t pAmps;
}sDaliIout_t;

typedef struct
{
    uint8_t addr;
    uint16_t pVolts;
}sDaliVout_t;

typedef struct
{
    uint8_t  addr;
    uint16_t temp;
}sDaliGetGearTemp_t;


typedef struct
{
  uint8_t addrToSet;
  uint8_t tuneVal;
}sCommission_t;
/**
 * @brief struct encoding the dali task type and pertinent information, to be expanded as more task types are added
 * 
 */
typedef struct
{
    eDaliTaskType_t eDaliTask;
    union 
    {
        sDaliDAPC_t         sSetDAPC    ;
        sDaliPwr_t          sGetPwr     ;
        sDaliNrg_t          sGetNrg     ;
        sDaliIout_t         sGetIout    ;
        sDaliVout_t         sGetVout    ;
        sDaliGetGearTemp_t  sGetGearTemp;
        sDaliReadMB_t       sDaliReadMB ;
        sDaliReadMB_t       sDaliWriteMB;
        sCommission_t       sCommission ;
        uint8_t             taskData[32];//Generic buffer
    }uTask;
}sDaliTask_t;

 
/**
 * @brief This typedef exists for the sole purpose of mirroring the payload of saDaliNetworkData_t to more easily get the size right for checksum calculation
 * 
 */
typedef struct
{
  uint8_t           numDrivers;
  uDaliDriverData_t uData[MAX_SUPPORTED_DRIVERS];
}sDaliNetworkPld_t;

/**
 * @brief struct data for persistent storage of DALI network information
 * 
 */
typedef struct
{
  uint8_t           numDrivers;
  uDaliDriverData_t uData[MAX_SUPPORTED_DRIVERS];
  uint8_t           checksum;
}saDaliNetworkData_t;


/*Function prototypes*/

/**
 * @brief Handles executing the scheduled DALI task, should continue to be called periodically until returns evDaliNoTaskRunning 
 * @param psDaliTask 
 * @return eDaliTaskStatus_t 
 */
eDaliTaskStatus_t daliManageTask      (void);

/**
 * @brief Set the Dali Task if not currently busy, copies internally and clears contents of psDaliTask if free. 
 * @param psDaliTask Task to assign.
 * @return _Bool 
 */
_Bool             setDaliTask         (sDaliTask_t * psDaliTask         );

/**
 * @brief Get the Currently running DALI Task 
 * @return eDaliTaskType_t 
 */
eDaliTaskType_t   getCurDaliTask      (void                             );

/**
 * @brief Returns true if a DALI task is running
 * @return _Bool true if DALI task is running
 */
_Bool             isDaliTaskRunning   (void                             );

/**
 * @brief copy static data to internal copy, verify contents 
 * @param psaDaliNetworkData pointer to  
 * @return _Bool true if copy is reasonably verified (checksum correct n num drivers reasonable)
 */
_Bool             initDaliStaticData  (const void  * psaDaliNetworkData );

/**
 * @brief Get the Address Of Dali Data  
 * @return void* 
 */
void *            getAddressOfDaliData(void                             );

/**
 * @brief initialize dali peripheral 
 */
void              initDALI            (void                             );

/**
 * @brief Get the Power Unit of dali at address, assuming the address is also the index! 
 * @param addr 
 */
float             getPowerUnit        (uint8_t addr                     );

/**
 * @brief Get the Energy Unit of dali at address (assuming the address is also the index!) 
 * @param addr 
 */
float             getEnergyUnit       (uint8_t addr                     );

/**
 * @brief Get the Dali type (DALI,D4i,Dexal,SR) 
 * @param addr 
 */
void              getDaliFlavor       (uint8_t addr, uint8_t *          );

/**
 * @brief Get the LED load wattage rating of the driver 
 * @param addr 
 * @return uint32_t 
 */
uint32_t          getRatedDaliWattage (uint8_t addr                     );

uint32_t          getDALIPower     (uint8_t index);
float             getDALIPowerFloat(uint8_t index);
uint32_t          getDALIEnergy    (uint8_t index);
uint16_t getDALILEDLoadVoltage(uint8_t index);
uint16_t getDALILEDLoadCurrent(uint8_t index);
uint16_t getDALiGearTemperature(uint8_t index);




