#include "dali_LED_Load.h"
#include "stdbool.h"


_Bool readDALILEDCurrent(sDaliDriverData_t * psDaliDriverStaticData, uint32_t * pAmps )
{
    switch(psDaliDriverStaticData->sStaticData.eDaliType)
    {
    case evD4i:
        if(true == getD4iLightSrcCurrent(psDaliDriverStaticData->sStaticData.addr,pAmps))
        {
        return true;
        }
    break;
    default:
        *pAmps = 0;
        return true;
    break;
    }
    
    return false;
}

_Bool readDALILEDVoltage(sDaliDriverData_t * psDaliDriverStaticData, uint32_t * pVolts )
{
    switch(psDaliDriverStaticData->sStaticData.eDaliType)
    {
    case evD4i:
        if(true == getD4iLightSrcVoltage(psDaliDriverStaticData->sStaticData.addr,pVolts))
        {
        return true;
        }
    break;
    default:
        *pVolts = 0;
        return true;
    break;
    }
    
    return false;
}

