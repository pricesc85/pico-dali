#include <stdint.h>

_Bool daliAssignAddress(uint8_t addr);
_Bool daliSingleAddressSequence(uint8_t addr);
_Bool daliTuneULTDriver(uint8_t addr, uint8_t tuneVal);
_Bool daliJCPHCommission(uint8_t addr, uint8_t tuneVal);
_Bool daliPollForControlGear(void);