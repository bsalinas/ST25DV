/**
  ******************************************************************************
  * @file    ST25DVSensor.h
  * @author  MCD Application Team
  * @brief   Header file of NFC ST25SV sensor module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#ifndef _ST25DVSENSOR_H_
#define _ST25DVSENSOR_H_
#include "Arduino.h"
#include "BSP/st25dv_nfctag.h"
#include "libNDEF/NDEFcommon.h"
#include "libNDEF/tagtype5_wrapper.h"
#include "libNDEF/lib_NDEF_URI.h"
#include "libNDEF/lib_NDEF_AAR.h"
#include "libNDEF/lib_NDEF_Bluetooth.h"
#include "libNDEF/lib_NDEF_Email.h"
#include "libNDEF/lib_NDEF_Handover.h"
#include "libNDEF/lib_NDEF_Geo.h"
#include "libNDEF/lib_NDEF_MyApp.h"
#include "libNDEF/lib_NDEF_SMS.h"
#include "libNDEF/lib_NDEF_Text.h"
#include "libNDEF/lib_NDEF_Vcard.h"
#include "libNDEF/lib_NDEF_Wifi.h"
#include <Wire.h>
#include <Stream.h>
#include "ST25DV/st25dv.h"

#define ST25DV_OK NDEF_OK
/**
 * @brief  ST25DV Ack Nack enumerator definition
 */
typedef enum {
  I2CANSW_ACK = 0,
  I2CANSW_NACK
} ST25DV_I2CANSW_E;

#if defined(ARDUINO_SAM_DUE)
#define WIRE Wire1
#else
#define WIRE Wire
#endif

//These are used to configure the interrupt
//GPO output level is controlled by Manage GPO Command (set/reset)
#define RF_USER_EN 0x01
// GPO output level changes from RF command EOF to response EOF
#define RF_ACTIVITY_EN 0x02
//GPO output level is controlled by Manage GPO Command (pulse)
#define RF_INTERRUPT_EN 0x04
//A pulse is emitted on GPO, when RF field appears or disappears.
#define FIELD_CHANGE_EN 0x08
//A pulse is emitted on GPO at completion of valid RF Write Message command.
#define RF_PUT_MSG_EN 0x10
//A pulse is emitted on GPO at completion of valid RF Read Message command if end of message has been reached
#define RF_GET_MSG_EN 0x20
// A pulse is emitted on GPO at completion of valid RF write operation in EEPROM
#define RF_WRITE_EN 0x40
//GPO output is enabled. GPO outputs enabled interrupts.
#define GPO_EN 0x80

//These are the possible values returned by getGPOInterruptStatus
//Manage GPO set GPO
#define RF_USER_STATE  0x01
// RF access
#define RF_ACTIVITY  0x02
//Manage GPO interrupt request
#define RF_INTERRUPT  0x04
// RF Field falling
#define FIELD_FALLING  0x08
// RF field rising
#define FIELD_RISING  0x10
// Message put by RF in FTM mailbox
#define RF_PUTMSG  0x20
// Message read by RF from FTM mailbox, and end of message has been reached.
#define RF_GETMSG  0x40
// Write in EEPROM
#define RF_WRITE  0x80


class ST25DV {
  public:
    ST25DV(void);
    int begin(uint8_t gpo, uint8_t ldp, TwoWire *pwire = &WIRE);
    int writeURI(String protocol, String uri, String info);
    int readURI(String *s);

    int writeText(char* text);
    //First, call findRecord.
    bool findRecord(sRecordInfo_t* record);
    NDEF_TypeDef getRecordType(sRecordInfo_t* record);
    int readURIFromRecord(sRecordInfo_t* record, String* s);
    int readTextFromRecord(sRecordInfo_t* record, String* s);

    void disableGPOInterrupt();
    void configureGPOInterrupt(uint16_t bitfield);
    //I have trouble calling this from the interrupt context.
    uint8_t getGPOInterruptReason();

    const char* typeString(NDEF_TypeDef typ);


    void ST25DV_GPO_Init(void);
    void ST25DV_GPO_DeInit(void);
    uint8_t ST25DV_GPO_ReadPin(void);
    void ST25DV_LPD_Init(void);
    void ST25DV_LPD_DeInit(void);
    uint8_t ST25DV_LPD_ReadPin(void);
    void ST25DV_LPD_WritePin(uint8_t LpdPinState);
    void ST25DV_I2C_Init(void);
    void ST25DV_SelectI2cSpeed(uint8_t i2cspeedchoice);

    TwoWire *_pwire;

  private:
    uint8_t _gpo;
    uint8_t _lpd;
};

extern ST25DV st25dv;
#endif