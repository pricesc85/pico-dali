/**
 * @file dali_commands.h
 * @author Scott Price (sprice@unvlt.com)
 * @brief Function declarations and data types related to DALI commands
 * @version 0.1
 * @date 2021-02-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once


#include <stdint.h>
/*enums*/

/**
 * @brief enum of all DALI standard command opcodes, format address:opcode
 */
typedef enum
{
    evOff                         = 0x00,/*!<Extinguish light output*/
    evUp                          = 0x01,/*!<Fade up for 200mS with set fade rate, interval reset by repeated command */
    evDown                        = 0x02,/*!<Fade down for 200mS with set fade rate, interval reset by repeated command*/
    evStepUp                      = 0x03,/*!<Step up one interval as quickly as possible, ignore if dim to off*/
    evStepDown                    = 0x04,/*!<Step down one interval as quickly as possible, don't dim to off*/
    evRecallMaxLevel              = 0x05,/*!<when initialisation state is enabled, ctrl gear sets actualLevel and targetLevel to maxLevel*/
    evRecallMinLevel              = 0x06,/*!<*/
    evStepDownAndOff              = 0x07,/*!<Same as step down, will dim to off*/
    evOnAndStepUp                 = 0x08,/*!<Same as step up, will come out of dim to off*/
    evEnableDAPCSequence          = 0x09,/*!<*/
    evGoToLastActiveLevel         = 0x0A,/*!<*/
    evContinuousUp                = 0x0B,/*!<dim up at set fade rate to max, no change if already max or 0*/
    evContinuousDown              = 0x0C,/*!<dim down at set fade rate to min, no change if already min or 0*/
    evGoToSceneX                  = 0x10,/*!<*/
    evReset                       = 0x20,/*!changes all variables to reset values. Control gear should react properly to subsequent commands within 300 mS<*/
    evStoreActualLevelInDTR0      = 0x21,/*!<store actualLevel in DTR0*/
    evSavePersistentVariables     = 0x22,/*!<Store table 14 variables in NVM, including extended NVM variables defined in parts 2xx. Responsive again within 300mS. For commissioning*/
    evSetOperatingModeToDTR0      = 0x23,/*!<Set operating mode to DTR0.  See subclause 9.9 for more info*/
    evResetMemoryBankDTR0         = 0x24,/*!<if DTR0 = 0, all implemented and unlocked memory banks except memory bank 0 shall be reset; if DTR0 != 0, the MB specified by DTR0 shall be reset (if implemented and unlocked)*/
    evIdentifyDevice              = 0x25,/*!<*/
    evSetMaxLevelToDTR0           = 0x2A,/*!<Set maxLevel to DTR0 unless DTR0 l.t.e minLevel; if DTR0 = MASK: 0xFE*/
    evSetMinLevelToDTR0           = 0x2B,/*!<set minLevel to DTR0, if DTR0 l.t.e PHM : PHM ; if DTR0 g.t.e maxLevel or MASK: maxLevel*/
    evSetSystemFailureLevelToDTR0 = 0x2C,/*!<system failure level set to DTR0. subclause 9.12 for more info*/
    evSetPowerOnLevelToDTR0       = 0x2D,/*!<powerOnLevel set to DTR0.  Subclause 9.13 for more info*/
    evSetFadeTimeToDTR0           = 0x2E,/*!<fade time set to DTR0; if DTR0 > 15: 15. If fade time is 0, extended fade time used*/
    evSetFadeRateToDTR0           = 0x2F,/*!<fade rate set to DTR0; if DTR0 > 15: 15. if DTR0 = 0:1.*/
    evSetExtendedFadeTimeToDTR0   = 0x30,/*!<if DTR0 > 0x4F, extendedFadeTimeBase = extendedFadeTimeMultiplier = 0 (fade as quickly as possible). Else, extended FadeTimeBase set to b3:b0 of DTR0, extendedFadeTimeMultiplier set to b6:b4 of DTR0*/
    evSetSceneXToDTR0             = 0x40,/*!<set Scene# to DTR0. See 9.19 for more info*/
    evRemoveFromSceneX            = 0x50,/*!<removed addressed control gear as a member from scene#. 9.19 for more info*/
    evAddToGroupX                 = 0x60,/*!<add addressed controlGear to group X*/
    evRemoveFromGroupX            = 0x70,/*!<remove addressed control gear from group X*/
    evSetShortAddressToDTR0       = 0x80,/*!<if DTR0 =  MASK:MASK (deletes short address); no change if DTR0 b7 = 1 or DTR0 b0 = 0; otherwise set short address to DTR0 b6:b1*/
    evEnableWriteMemory           = 0x81,/*!<set writeEnableState to ENABLED. Any command not directly involved with writing memory banks will set writeEnableStatus to disabled. Valid commands to keep enabled: write memory location, write memory location no reply, write dtr0, dtr1, dtr2 0 See 9.10.5*/
    evQueryStatus                 = 0x90,/*!<Query control gear status, 9.16 for more info*/
    evQueryControlGearPresent     = 0x91,/*!<if control gear is present, responds with YES*/
    evQueryLampFailure            = 0x92,/*!<Answer is YES if lampFailure is TRUE and NO otherwise*/
    evQueryLampPowerOn            = 0x93,/*!<Answer is YES if lampOn is TRUE and NO otherwise*/
    evQueryLimitError             = 0x94,/*!<Answer is YES if limitError is TRUE and NO otherwise*/
    evQueryResetState             = 0x95,/*!<Answer is YES if resetState is TRUE and NO otherwise*/
    evQueryMissingShortAddress    = 0x96,/*!<Answer is yes if shortAddress is equal to MASK and NO otherwise. NOTE, since ctrl gear answers only if no short address is stored, only useful in broadcast mode or if group addressing used*/
    evQueryVersionNumber          = 0x97,/*!<Answer shall be content of memory bank 0 location 0x16*/
    evQueryContentDTR0            = 0x98,/*!<Response is contents of DTR0*/
    evQueryDeviceType             = 0x99,/*!<If no Part 2xx is implemented, answer is 254; if one device type/feature supported, answer is device type/feature #; if multiple, answer is MASK. 9.8,11.5.13 for more info*/
    evQueryPhysicalMinimum        = 0x9A,/*!<Response is physical minimum (PHM)*/
    evQueryPowerFailure           = 0x9B,/*!<YES if powerCycleSeen is true; no otherwise*/
    evQueryContentDTR1            = 0x9C,/*!<Response is contents of DTR1*/
    evQueryContentDTR2            = 0x9D,/*!<Response is contents of DTR2*/
    evQueryOperatingMode          = 0x9E,/*!<Response is operating mode. 9.9 for more info*/
    evQueryLightSourceType        = 0x9F,/*!<Response shall be light source type #, specified in Table 17. If answer is MASK, DTR0 will be first type, DTR1 will be second type, DTR2 will be third type*/
    evQueryActualLevel            = 0xA0,/*!<Response is actualLevel; MASK during startup or no light output while output is expected*/
    evQueryMaxLevel               = 0xA1,/*!<response is maxLevel*/
    evQueryMinLevel               = 0xA2,/*!<response is minLevel*/
    evQueryPowerOnLevel           = 0xA3,/*!<response is powerOnLevel. 9.12 for more info*/
    evQuerySystemFailureLevel     = 0xA4,/*!<Response is sytemFailureLevel. 9.12 for more info*/
    evQueryFadeTimeAndRate        = 0xA5,/*!<*/
    evQueryMfgSpecificMode        = 0xA6,/*!<*/
    evQueryNextDeviceType         = 0xA7,/*!<Reports next highest numbered supported device type after each query; should follow a first query device type whose response was 254; if all types reported, response is 254; response NO in all other cases. 9.18 and 11.5.12*/
    evQueryExtendedFadeTime       = 0xA8,/*!<Response is 0b0xxxyyyy, where xxx equals extendedFadeTimeMultiplier, and yyyy is extendedFadeTimeBase*/
    evQueryControlGearFailure     = 0xAA,/*!<response is YES if controlGearFailure is TRUE and NO otherwise*/
    evQuerySceneXLevel            = 0xB0,/*!<Answer shall be scene#; refer to 9.19 for more info*/
    evQueryGroups0To7             = 0xC0,/*!<answer shall be gearGroups[7:0] a bitfield where each bit is set if gear is a member of the group*/
    evQueryGroups8To15            = 0xC1,/*!<Same as above for gearGroups[8:15]*/
    evQueryRandomAddrH            = 0xC2,/*!<Query high byte of random address*/
    evQueryRandomAddrM            = 0xC3,/*!<Query mid byte of random address*/
    evQueryRandomAddrL            = 0xC4,/*!<Query low byte of random address*/
    evReadMemoryBank              = 0xC5,/*!<Discarded if specified memory bank not implemented; if implemented, response is memory location DTR0 in memory bank DTR1; DTR0 will increment by 1 if < FF. 9.10 for more info*/
    evQueryExtendedVersionNum     = 0xFF /*!<Response is version # of part 2xx for corresponding device type/feature. No if not implemented. 9.18 for more info*/
}eDaliStandardCommands_t;

/**
 * @brief enum of all DALI special command opcodes, format opcode:data/device/0x00
 */
typedef enum
{
    evTerminate          = 0xA1,/*!<Terminates initialisation(initialisationState set to DISABLED), Identification, other processes identified in 2xx parts, 9.14.2 for more info*/
    evSetDTR0            = 0xA3,/*!<Set DTR0 to data(opcode byte)*/
    evInitialise         = 0xA5,/*!<device = 0b0AAAAAA1, device with short addr 0b00AAAAAA will initialize; device = 0b11111111, unaddressed controlGear will react; 0b00000000, all control gear will react; 9.14.2*/
    evRandomise          = 0xA7,/*!<discarded if initialisationState DISABLED; otherwise chall generate randomAddress b/t 0 and 0xFFFFFE*/
    evCompare            = 0xA9,/*!<discarded unless initialisationState is ENABLED; respond yes if randomAddress <= searchAddress; no response otherwise*/
    evWithdraw           = 0xAB,/*!<Discarded unless following conditions hold: initialisationState ENABLED, randomAdress = searchAddress; if executed, controlGear set initialisationState to WITHDRAWN; application control may/ should assign a short address prior to this cmd*/
    evPing               = 0xAD,/*!<Used by single master application controllers to indicate their presence. Ignored by control gear*/
    evSearchAddrH        = 0xB1,/*!<Discarded if initialisationState DISABLED; if executed, searchAddr[23:16] set to data*/
    evSearchAddrM        = 0xB3,/*!<Discarded if initialisationState DISABLED; if executed, searchAddr[15:8] set to data*/
    evSearchAddrL        = 0xB5,/*!<Discarded if initialisationState DISABLED; if executed, searchAddr[7:0] set to data*/
    evprogramShortAddr   = 0xB7,/*!<Discarded unless initialisationState set to ENABLED or WITHDRAWN, and randomAddress equal to searchAddress; if executed, shortAddress set: if data = MASK: MASK (deletes short addr), if data = 0b1xxxxxxx or 0bxxxxxxx0: no change, if data = 0b0AAAAAA1 short addr = 0b00AAAAAA*/
    evVerifyShortAddr    = 0xB9,/*!<discarded if initialisationState DISABLED. If executed, answer yes if short address equals 0b00AAAAAA for data 0b0AAAAAA1*/
    evQueryShortAddr     = 0xBB,/*!<Discarded if initialisationState DISABLED or randomAddress != searchAddress; answer sdhall be 0b0AAAAAA1 where shortAddress = 0b00AAAAAA, or MASK if shortAddress = MASK*/
    evEnableDeviceType   = 0xC1,/*!<Select the device type/feature for which the next application extended command is valid; cancels any previous selection of a device type/feature; only valid for next application extended command*/
    evSetDTR1            = 0xC3,/*!<Set DTR1 to data(opcode byte)*/
    evSetDTR2            = 0xC5,/*!<Set DTR2 to data(opcode byte)*/
    evWriteMemoryBank    = 0xC7,/*!<Discarded if addressed memory bank not implemented or writeEnableState is DISABLED; writes data to memory location DTR0 in memory bank DTR1, returns data as answer; broadcast command, addressing a controlGear achieved by setting write enable condition on select controlGear; DTR0 gets post-incremented if l.t. 255*/
    evWriteMemBnkNoReply = 0xC9 /*!<Same as write memory location with no reply*/
}eDaliSpecialCommands_t;

/**
 * @brief enum specifying which command type is to be sent
 */
typedef enum
{
    evStandard,/*!<Command is in the 'opcode' byte*/
    evSpecial ,/*!<Command is in the 'address' byte*/
    evFeedThru,/*!<Feed command through without checking*/
    evDAPC     /*!<Special bit set in address byte */  
}eDaliCommandType_t;


/**
 * @brief enum specifying the addressing type for the command to be sent
 */
typedef enum
{
evShortAddress        ,/*!<A single 0-63 short address*/    
evGroupAddress        ,/*!<Device belonging to 1 of 16 groups */
evBroadcastUnaddressed,/*!<All unaddressed devices*/
evBroadcastAll         /*!<All devices*/
}eDaliStandardAddressType_t;

/**
 * @brief enum specifying the payload type
 */
typedef enum
{
    eNull  ,/*!<2nd byte is zero, ignore payload byte*/
    eData  ,/*!<2nd byte is data, pass to forward frame directly*/
    eDevice /*!<2nd byte is a device, decorate accordingly*/
}eDaliSpecialPayloadType_t;




void generateForwardFrame     (uint16_t * pFFrame                  );

/**
 * @brief Helper function for encoding the address information based on address type and address specified
 * 
 * @param eAddrType 
 * @param addr 
 * @param dest 
 */
void generateAddr             (eDaliStandardAddressType_t eAddrType,
                               uint8_t                    addr     ,     
                               uint8_t                   *dest     );

/**
 * @brief Sends command to tell driver(s) to go to a level 0x00-0xFE
 * 
 * @param addr 
 * @param eAddrType 
 * @param lvl 
 */
void sendCmdDapc              (uint8_t                    addr     ,
                               eDaliStandardAddressType_t eAddrType,
                               uint8_t                    lvl      );

/**
 * @brief Prepare forward frame for transmission of standard comand with no reply
 * 
 * @param addr 
 * @param eAddrType 
 * @param eStandardCmd 
 */
void sendStandardCmdNoReply   (uint8_t addr                        ,
                               eDaliStandardAddressType_t eAddrType,
                               eDaliStandardCommands_t eStandardCmd);

/**
 * @brief Prepare the forward frame for transmission of standard command with reply expected
 * 
 * @param addr 
 * @param eAddrType 
 * @param eCmd 
 */
void  sendStandardCmdWithReply(uint8_t addr                        , 
                               eDaliStandardAddressType_t eAddrType,
                               eDaliStandardCommands_t             );

/**
 * @brief prepare the forward frame for transmission of send twice standard command
 * 
 * @param addr 
 * @param eAddrType 
 * @param eStandardCmd 
 */
void  sendStandardCmdTwice    (uint8_t addr                        ,
                               eDaliStandardAddressType_t eAddrType,
                               eDaliStandardCommands_t eStandardCmd);

/**
 * @brief prepare the forward frame for transmisstion of special command with reply expected
 * 
 * @param data 
 * @param eSpecialCmd 
 */
void  sendSpecialCmdWithReply  (uint8_t data                        ,
                               eDaliSpecialCommands_t eSpecialCmd  );

/**
 * @brief Prepare the forward frame for transmission of a send once special command
 * 
 * @param data 
 * @param eSpecialCmd 
 */
void  sendSpecialCmdNoReply   (uint8_t                data         ,
                               eDaliSpecialCommands_t eSpecialCmd  );

/**
 * @brief Prepare the forward frame for transmission of a send twice special command
 * 
 * @param data 
 * @param eSpecialCmd 
 */
void  sendSpecialCmdTwice     (uint8_t addr                        ,
                               eDaliSpecialCommands_t eSpecialCmd  );

