#include "dali_temperature.h"
#include "dali_d4i.h"


_Bool readDALIDriverTemperature     (sDaliDriverData_t * psDaliDriverStaticData, uint32_t * pTmp )
{
    switch(psDaliDriverStaticData->sStaticData.eDaliType)
    {
    case evD4i:
        if(true == getD4iGearTemperature(psDaliDriverStaticData->sStaticData.addr,pTmp))
        {
        return true;
        }
    break;
    default:
        *pTmp = 0;
        return true;
    break;
    }
    
    return false;
}