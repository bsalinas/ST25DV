/**
  ******************************************************************************
  * @file    ST25DVSensor.c
  * @author  MCD Application Team
  * @brief   Source file of NFC ST25SV sensor module.
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

#include "ST25DVSensor.h"

ST25DV st25dv;

Stream *_serial = NULL;

ST25DV::ST25DV(void)
{
}

NFCTAG_StatusTypeDef ST25DV_IO_Init(void);
NFCTAG_StatusTypeDef ST25DV_IO_MemWrite(const uint8_t *const pData, const uint8_t DevAddr, const uint16_t TarAddr, const uint16_t Size);
NFCTAG_StatusTypeDef ST25DV_IO_MemRead(uint8_t *const pData, const uint8_t DevAddr, const uint16_t TarAddr, const uint16_t Size);
NFCTAG_StatusTypeDef ST25DV_IO_Read(uint8_t *const pData, const uint8_t DevAddr, const uint16_t Size);
NFCTAG_StatusTypeDef ST25DV_IO_IsDeviceReady(const uint8_t DevAddr, const uint32_t Trials);
NFCTAG_StatusTypeDef NFCTAG_ConvertStatus(uint8_t ret);

int ST25DV::begin(uint8_t gpo, uint8_t lpd, TwoWire *pwire)
{
  int ret = NDEF_OK;

  _pwire = pwire;

  _gpo = gpo;
  _lpd = lpd;

  ret = BSP_NFCTAG_Init();
  if (ret != NDEF_OK) {
    return ret;
  }

  BSP_NFCTAG_GetExtended_Drv()->ResetMBEN_Dyn();
  if (NfcType5_NDEFDetection() != NDEF_OK) {
    CCFileStruct.MagicNumber = NFCT5_MAGICNUMBER_E1_CCFILE;
    CCFileStruct.Version = NFCT5_VERSION_V1_0;
    CCFileStruct.MemorySize = (ST25DV_MAX_SIZE / 8) & 0xFF;
    CCFileStruct.TT5Tag = 0x05;
    /* Init of the Type Tag 5 component (M24LR) */
    ret = NfcType5_TT5Init();
    if (ret != NDEF_OK) {
      return ret;
    }
  }

  return ret;
}

bool ST25DV::findRecord(sRecordInfo_t* record)
{
  if(record == NULL) return false;
  int ret = NDEF_ReadNDEF(NDEF_Buffer);
  if (ret) {
    if(_serial)
    {
        _serial->print("Err1:");
        _serial->println(ret);
    }
    return false;
  }

  ret = NDEF_IdentifyBuffer(record, NDEF_Buffer);
  if (ret) {
    if(_serial)
    {
        _serial->print("Err2:");
        _serial->println(ret);
    }
    return false;
  }
  return true;

}
const char* ST25DV::typeString(NDEF_TypeDef typ)
{
  switch(typ)
  {
    case TEXT_TYPE:
    {
        return "TEXT_TYPE";
    }
    case URL_TYPE:
    {
        return "URL_TYPE";
    }
    case UNKNOWN_TYPE:
    {
        return "UNKNOWN_TYPE";
    }
    case VCARD_TYPE:
    {
        return "VCARD_TYPE";
    }
    case WELL_KNOWN_ABRIDGED_URI_TYPE:
    {
        return "WELL_KNOWN_ABRIDGED_URI_TYPE";
    }
    case URI_SMS_TYPE:
    {
        return "URI_SMS_TYPE";
    }
    case URI_GEO_TYPE:
    {
        return "URI_GEO_TYPE";
    }
    case URI_EMAIL_TYPE:
    {
        return "URI_EMAIL_TYPE";
    }
    case SMARTPOSTER_TYPE:
    {
        return "SMARTPOSTER_TYPE";
    }
    case HANDOVER_TYPE:
    {
        return "HANDOVER_TYPE";
    }
    case M24SR_DISCOVERY_APP_TYPE:
    {
        return "M24SR_DISCOVERY_APP_TYPE";
    }
    case BT_TYPE:
    {
        return "BT_TYPE";
    }
    case BLE_TYPE:
    {
        return "BLE_TYPE";
    }
    case URI_WIFITOKEN_TYPE:
    {
        return "URI_WIFITOKEN_TYPE";
    }
  }
}
NDEF_TypeDef ST25DV::getRecordType(sRecordInfo_t* record)
{
  if(record == NULL) return UNKNOWN_TYPE;

  return record->NDEF_Type;
}
int ST25DV::readURIFromRecord(sRecordInfo_t* record, String* s)
{
  if(record == NULL) return NFCTAG_ERROR;

  sURI_Info uri = {"", "", ""};

  int ret = NDEF_ReadURI(record, &uri);
  if (ret) {
    return ret;
  }
  *s = String(uri.protocol) + String(uri.URI_Message);

  return NFCTAG_OK;
}
int ST25DV::readTextFromRecord(sRecordInfo_t* record, String* s)
{
  if(record == NULL) return NFCTAG_ERROR;
  NDEF_Text_info_t text;
  int ret = NDEF_ReadText(record, &text);
  if(ret)
  {
    return ret;
  }
  *s = String(text.text);
  return NFCTAG_OK;
}

int ST25DV:: writeText(char* text)
{
  return NDEF_WriteText(text);
}
int ST25DV::writeURI(String protocol, String uri, String info)
{
  sURI_Info _URI;
  strcpy(_URI.protocol, protocol.c_str());
  strcpy(_URI.URI_Message, uri.c_str());
  strcpy(_URI.Information, info.c_str());

  return NDEF_WriteURI(&_URI);
}



int ST25DV::readURI(String *s)
{
  uint16_t ret;
  sURI_Info uri = {"", "", ""};
  sRecordInfo_t recordInfo;
  // increase buffer size for bigger messages
  ret = NDEF_ReadNDEF(NDEF_Buffer);
  if (ret) {
    if(_serial)
    {
        _serial->print("Err1:");
        _serial->println(ret);
    }
    return ret;
  }

  ret = NDEF_IdentifyBuffer(&recordInfo, NDEF_Buffer);
  if (ret) {
    if(_serial)
    {
        _serial->print("Err2:");
        _serial->println(ret);
    }
    return ret;
  }

  ret = NDEF_ReadURI(&recordInfo, &uri);
  if (ret) {
    if(_serial)
    {
        _serial->print("Err3:");
        _serial->println(ret);
    }
    return ret;
  }
  *s = String(uri.protocol) + String(uri.URI_Message);

  return 0;
}
void ST25DV::disableGPOInterrupt()
{
  BSP_NFCTAG_ConfigIT(0);
}
void ST25DV::configureGPOInterrupt(uint16_t bitfield)
{
  const ST25DV_PASSWD defaultPW = (const ST25DV_PASSWD){0,0};
  
  int ret = St25Dv_i2c_ExtDrv.PresentI2CPassword(defaultPW);
  if(ret != 0)
  {
    Serial.println("Password Failed");
  }
  uint16_t new_bitfield = bitfield;
  //if we are enabling, we want to always set GPO_EN
  new_bitfield = new_bitfield | GPO_EN;
  ret = BSP_NFCTAG_ConfigIT(new_bitfield);
  Serial.print("CONFIG:");
  Serial.println(ret);
}
uint8_t ST25DV::getGPOInterruptReason()
{
  uint8_t status = 0;
  int ret = St25Dv_i2c_ExtDrv.ReadITSTStatus_Dyn(&status);
  Serial.print("GETIT:");
  Serial.print(ret);
  Serial.print(",");
  Serial.println(status);
  return status;
}

/**
  * @brief  This function initialize the GPIO to manage the NFCTAG GPO pin
  * @param  None
  * @retval None
  */
void ST25DV::ST25DV_GPO_Init(void)
{
  pinMode(_gpo, INPUT);
}

/**
  * @brief  This function get the GPIO value through GPIO
  * @param  None
  * @retval HAL GPIO pin status
  */
uint8_t ST25DV::ST25DV_GPO_ReadPin(void)
{
  return digitalRead(_gpo);
}

/**
  * @brief  This function initialize the GPIO to manage the NFCTAG LPD pin
  * @param  None
  * @retval None
  */
void ST25DV::ST25DV_LPD_Init(void)
{
  pinMode(_lpd, OUTPUT);
  digitalWrite(_lpd, LOW);
}

/**
  * @brief  DeInit LPD.
  * @param  None.
  * @note LPD DeInit does not disable the GPIO clock nor disable the Mfx
  */
void ST25DV::ST25DV_LPD_DeInit(void)
{
}

/**
  * @brief  This function get the GPIO value through GPIO
  * @param  None
  * @retval HAL GPIO pin status
  */
uint8_t ST25DV::ST25DV_LPD_ReadPin(void)
{
  return digitalRead(_lpd);
}

/**
  * @brief  This function get the GPIO value through GPIO
  * @param  None
  * @retval HAL GPIO pin status
  */
void ST25DV::ST25DV_LPD_WritePin(uint8_t LpdPinState)
{
  digitalWrite(_lpd, LpdPinState);
}

/**
  * @brief  This function select the I2C1 speed to communicate with NFCTAG
  * @param  i2cspeedchoice Number from 0 to 5 to select i2c speed
  * @retval HAL GPIO pin status
  */
void ST25DV::ST25DV_SelectI2cSpeed(uint8_t i2cspeedchoice)
{
  if (_pwire == NULL) {
    return;
  }

#if !defined(ARDUINO_ARCH_ARC) && !defined(ARDUINO_ARCH_ARC32)
  switch (i2cspeedchoice) {
    case 0:

      _pwire->setClock(10000);
      break;

    case 1:

      _pwire->setClock(100000);
      break;

    case 2:

      _pwire->setClock(200000);
      break;

    case 3:

      _pwire->setClock(400000);
      break;

    case 4:

      _pwire->setClock(800000);
      break;

    case 5:

      _pwire->setClock(1000000);
      break;

    default:

      _pwire->setClock(1000000);
      break;
  }
#endif
}

/**
  * @brief  This function initialize the I2C
  * @param  None
  * @retval None
  */
void ST25DV::ST25DV_I2C_Init(void)
{
  _pwire->begin();
}

/******************************** LINK EEPROM COMPONENT *****************************/

/**
  * @brief  Initializes peripherals used by the I2C NFCTAG driver
  * @param  None
  * @retval NFCTAG enum status
  */
NFCTAG_StatusTypeDef ST25DV_IO_Init(void)
{
  if (st25dv._pwire == NULL) {
    return NFCTAG_ERROR;
  }

  st25dv.ST25DV_GPO_Init();
  st25dv.ST25DV_LPD_Init();

  st25dv.ST25DV_I2C_Init();
  st25dv.ST25DV_SelectI2cSpeed(3);

  return NFCTAG_OK;
}

/**
  * @brief  Write data, at specific address, through i2c to the ST25DV
  * @param  pData: pointer to the data to write
  * @param  DevAddr : Target device address
  * @param  TarAddr : I2C data memory address to write
  * @param  Size : Size in bytes of the value to be written
  * @retval NFCTAG enum status
  */
NFCTAG_StatusTypeDef ST25DV_IO_MemWrite(const uint8_t *const pData, const uint8_t DevAddr, const uint16_t TarAddr, const uint16_t Size)
{
  NFCTAG_StatusTypeDef pollstatus;
  byte ret;
  uint32_t tickstart;
  uint8_t Addr = DevAddr >> 1;
  char tmp[4];

  if (st25dv._pwire == NULL) {
    return NFCTAG_ERROR;
  }

  if (_serial != NULL) {
    //  _serial->println(SP);
    _serial->print("  w ");
    sprintf(tmp, "%02X", Addr);
    _serial->print(tmp);
    _serial->print("@");
    sprintf(tmp, "%02X", TarAddr >> 8);
    _serial->print(tmp);
    sprintf(tmp, "%02X", TarAddr & 0xFF);
    _serial->print(tmp);
    _serial->print(":");
    _serial->println(Size);
    _serial->print("  ");
    for (uint16_t d = 0; d < Size; d++) {
      sprintf(tmp, "%02X", pData[d]);
      _serial->print(tmp);
    }
    _serial->println("");
  }

  st25dv._pwire->beginTransmission(Addr); // transmit to device
  st25dv._pwire->write(TarAddr >> 8);   // send memory address MSB
  st25dv._pwire->write(TarAddr & 0xFF); // send memory address LSB
  st25dv._pwire->write(pData, Size);        // sends  bytes
  ret = st25dv._pwire->endTransmission(true);    // stop transmitting
  if (_serial != NULL) {
    _serial->print("  =");
    _serial->println(ret);
  }

  if (ret == 0) {
    /* Poll until EEPROM is available */
    tickstart = millis();
    /* Wait until ST25DV is ready or timeout occurs */
    do {
      pollstatus = ST25DV_IO_IsDeviceReady(DevAddr, 1);
    } while (((millis() - tickstart) < ST25DV_I2C_TIMEOUT) && (pollstatus != NFCTAG_OK));

    if (pollstatus != NFCTAG_OK) {
      return NFCTAG_TIMEOUT;
    }
  }
#if defined(ARDUINO_ARCH_ARC) || defined(ARDUINO_ARCH_ARC32)
  // Arduino 101 i2c seems buggy after an address NACK: restart the i2c
  else if (ret == 2) {
    if (_serial != NULL) {
      _serial->print("  -\n");
    }
    st25dv._pwire->begin();
  }
#endif
  return NFCTAG_ConvertStatus(ret);
}

/**
  * @brief  Reads data at a specific address from the NFCTAG.
  * @param  pData: pointer to store read data
  * @param  DevAddr : Target device address
  * @param  TarAddr : I2C data memory address to read
  * @param  Size : Size in bytes of the value to be read
  * @retval NFCTAG enum status
  */
NFCTAG_StatusTypeDef ST25DV_IO_MemRead(uint8_t *const pData, const uint8_t DevAddr, const uint16_t TarAddr, const uint16_t Size)
{
  int i = 0;
  uint8_t ret = 4;
  uint8_t Addr = DevAddr >> 1;
  char tmp[4];

  if (st25dv._pwire == NULL) {
    return NFCTAG_ERROR;
  }

  if (_serial != NULL) {
    //  _serial->println(SP);
    _serial->print("  r ");
    sprintf(tmp, "%02X", Addr);
    _serial->print(tmp);
    _serial->print("@");
    sprintf(tmp, "%02X", TarAddr >> 8);
    _serial->print(tmp);
    sprintf(tmp, "%02X", TarAddr & 0xFF);
    _serial->print(tmp);
    _serial->print(":");
    _serial->println(Size);
  }
  st25dv._pwire->beginTransmission(Addr);    // Get the slave's attention, tell it we're sending a command byte
  st25dv._pwire->write(TarAddr >> 8);           //  The command byte, sets pointer to register with address of 0x32
  st25dv._pwire->write(TarAddr & 0xFF);         //  The command byte, sets pointer to register with address of 0x32
  ret = st25dv._pwire->endTransmission(true);
  // Address is not OK
  if (ret != 0) {
    if(_serial)
    {
      _serial->print("ret_early:");
      //Getting a BUS_BUSY error here.
      _serial->println(ret);
    }
    return NFCTAG_ConvertStatus(ret);
  }

  // be carefull with the (int)Size, the size parameter us no more on 16 bits but only 15 bits (the cast is required for arduino UNO)
  st25dv._pwire->requestFrom((int)Addr, (int)Size); // Tell slave we need to read 1byte from the current register


  while (st25dv._pwire->available()) {
    pData[i++] = st25dv._pwire->read();      // read that byte into 'slaveByte2' variable
  }
  if (_serial != NULL) {
    _serial->print("  ");
    for (int d = 0; d < i; d++) {
      sprintf(tmp, "%02X", pData[d]);
      _serial->print(tmp);
    }
    _serial->println("");
  }

  /*
  It doesn't seem like Arduino wants you to call `endTransmission` 
  after `requestFrom`. On the ESP32 the ret value is 8 because it
  is an `endTransmission` without a `startTransmission` which is 
  considered an error by the library.
  */
  // ret = st25dv._pwire->endTransmission();
  // if (_serial != NULL) {
  //   //  _serial->println(pData[0]);
  //   _serial->print("  =");
  //   _serial->println(ret);
  //   //  _serial->print("  ");
  //   //  _serial->println(pData[0]);
  //   //  _serial->println((uint32_t)pData);
  // }
  return NFCTAG_ConvertStatus(ret);
}

NFCTAG_StatusTypeDef NFCTAG_ConvertStatus(uint8_t ret)
{
  if (ret == 0) {
    return NFCTAG_OK;
  } else if ((ret == 2) || (ret == 3)) {
    return NFCTAG_NACK;
  } else {
    return NFCTAG_ERROR;
  }
}


/**
  * @brief  Reads data at current address from the NFCTAG.
  * @param  pData: pointer to store read data
  * @param  DevAddr : Target device address
  * @param  Size : Size in bytes of the value to be read
  * @retval NFCTAG enum status
  */
NFCTAG_StatusTypeDef ST25DV_IO_Read(uint8_t *const pData, const uint8_t DevAddr, const uint16_t Size)
{
  int i = 0;
  uint8_t Addr = DevAddr >> 1;
  char tmp[4];

  if (st25dv._pwire == NULL) {
    return NFCTAG_ERROR;
  }
  if (_serial != NULL) {
    _serial->print("  r");
    _serial->print(":");
    _serial->println(Size);
  }
  // be carefull with the (int)Size, the size parameter us no more on 16 bits but only 15 bits (the cast is required for arduino UNO)
  byte ret = st25dv._pwire->requestFrom((int)Addr, (int)Size); // Tell slave we need to read 1byte from the current register
  while (st25dv._pwire->available()) {
    pData[i++] = st25dv._pwire->read();      // read that byte into 'slaveByte2' variable
  }

  if (_serial != NULL) {
    _serial->print("  ");
    for (int d = 0; d < i; d++) {
      sprintf(tmp, "%02X", pData[d]);
      _serial->print(tmp);
    }
    _serial->println("");
  }
  /*
  It doesn't seem like Arduino wants you to call `endTransmission` 
  after `requestFrom`. On the ESP32 the ret value is 8 because it
  is an `endTransmission` without a `startTransmission` which is 
  considered an error by the library.
  */
  // ret = st25dv._pwire->endTransmission();

  // if (_serial != NULL) {
  //   //  _serial->println(pData[0]);
  //   _serial->print("  =");
  //   _serial->println(ret);
  // }
  return NFCTAG_ConvertStatus(ret);
}


/**
* @brief  Checks if target device is ready for communication
* @note   This function is used with Memory devices
* @param  DevAddr : Target device address
* @retval NFCTAG enum status
*/
NFCTAG_StatusTypeDef ST25DV_IO_IsDeviceReady(const uint8_t DevAddr, const uint32_t Trials)
{
  int ret = 4;
  uint32_t count = 0;
  if (st25dv._pwire == NULL) {
    return NFCTAG_ERROR;
  }
  if (_serial != NULL) {
    _serial->println("  ?");
  }

  while ((count++ < Trials) && ret) {
    st25dv._pwire->beginTransmission(DevAddr >> 1);
    ret = st25dv._pwire->endTransmission();
    if (_serial != NULL) {
      _serial->print("  =");
      _serial->println(ret);
    }
  }
  return NFCTAG_ConvertStatus(ret);
}

