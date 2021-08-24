/**
******************************************************************************
* How to use this sketch
*
* This sketch uses the I2C interface to communicate with the NFC device.
* If write is enabled, it will 
*
* When the NFC module is started and ready, the message "Sytstem init done!" is
* displayed on the monitor window. Next, the tag is written, read and printed on
* the monitor window.
*
* You can also use your smartphone to read/write a tag.
* On Android, download a NFC Tools. Then start the app, check if NFC is activated
* on your smartphone. Put your smartphone near the tag, you can read it. You can
* write a tag with this app.
******************************************************************************
*/
 
#include "ST25DVSensor.h"

#define SerialPort      Serial

#define WireNFC Wire // Default wire instance

#define GPO_PIN 32
#define LPD_PIN A0

#if !defined(GPO_PIN) || !defined(LPD_PIN)
#error define the pin and wire instance used for your board
#endif


#define WRITE_TYPE_NONE 0
#define WRITE_TYPE_TEXT 1
#define WRITE_TYPE_URL 2
//for the example, if you want it to first write a record, put that type below.
#define SELECTED_WRITE_TYPE WRITE_TYPE_NONE
//#define SELECTED_WRITE_TYPE WRITE_TYPE_TEXT
//#define SELECTED_WRITE_TYPE WRITE_TYPE_URL


//This will hold information about the record found
sRecordInfo_t m_recordInfo;

void setup() {
//  Wire.begin();

  String read_data;

  // Initialize serial for output.
  SerialPort.begin(115200);

  // The wire instance used can be omited in case you use default Wire instance
  if(st25dv.begin(GPO_PIN, LPD_PIN) == 0) {
    SerialPort.println("System Init done!");
  } else {
    SerialPort.println("System Init failed!");
    while(1);
  }
 int ret = 0;
 #if SELECTED_WRITE_TYPE == WRITE_TYPE_URL
  Serial.println("Writing a URL to the tag");
  const char uri_write_message[] = "st.com/st25";       // Uri message to write in the tag
  const char uri_write_protocol[] = URI_ID_0x01_STRING; // Uri protocol to write in the tag
  String uri_write = String(uri_write_protocol) + String(uri_write_message);
  Serial.println(uri_write);
  ret = st25dv.writeURI(uri_write_protocol, uri_write_message, "");
  if(ret != 0) {
    SerialPort.print("Write failed:");
    SerialPort.println(ret);
    while(1);
  }
  delay(100);
#endif
#if SELECTED_WRITE_TYPE == WRITE_TYPE_TEXT
  Serial.println("Writing Text to the tag");
  ret = st25dv.writeText("Hello World!");
  if(ret != 0)
  {
    Serial.print("Write failed:");
    Serial.println(ret);
    while(1);
  }
  
  delay(100);
#endif

  if(st25dv.findRecord(&m_recordInfo))
  {
    Serial.println("\nFound a record");
    NDEF_TypeDef typ = st25dv.getRecordType(&m_recordInfo);
    Serial.println(st25dv.typeString(typ));
    switch(typ)
    {
      case TEXT_TYPE:
      {
        Serial.print("TEXT:\"");
        ret = st25dv.readTextFromRecord(&m_recordInfo, &read_data);
        if(ret)
        {
          Serial.print("ERROR:");         
          Serial.println(ret);
        } else
        {
          Serial.print(read_data);
          Serial.println("\"");
        }
        break;
      }
      case WELL_KNOWN_ABRIDGED_URI_TYPE:
      {
        int ret = st25dv.readURIFromRecord(&m_recordInfo, &read_data);
        if(ret)
        {
          Serial.print("ERROR:");
          Serial.println(ret);
        } else
        {
          Serial.print("URL:\"");
          Serial.print(read_data);
          Serial.println("\"");  
        }
          break;
      }
      case UNKNOWN_TYPE:
      case VCARD_TYPE:
      case URI_SMS_TYPE:
      case URI_GEO_TYPE:
      case URI_EMAIL_TYPE:
      case URL_TYPE:
      case SMARTPOSTER_TYPE:
      case HANDOVER_TYPE:
      case M24SR_DISCOVERY_APP_TYPE:
      case BT_TYPE:
      case BLE_TYPE:
      case URI_WIFITOKEN_TYPE:
      {
        break;
      }
     }
  } else
  {
    Serial.println("\nNo Records");
  }
}

void loop() {  
  //empty loop
} 
