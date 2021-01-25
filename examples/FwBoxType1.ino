//
// Copyright (c) 2020 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//
// Connections :
//
// Required Library :
//

#include "FwBox.h"
#include <Wire.h>


#define DEVICE_TYPE 1
#define FIRMWARE_VERSION "1.1"

#define VAL_INDEX_LED 0


//
// Debug definitions
//
#define FW_BOX_DEBUG 1

#if FW_BOX_DEBUG == 1
  #define DBG_PRINT(VAL) Serial.print(VAL)
  #define DBG_PRINTLN(VAL) Serial.println(VAL)
  #define DBG_PRINTF(FORMAT, ARG) Serial.printf(FORMAT, ARG)
  #define DBG_PRINTF2(FORMAT, ARG1, ARG2) Serial.printf(FORMAT, ARG1, ARG2)
#else
  #define DBG_PRINT(VAL)
  #define DBG_PRINTLN(VAL)
  #define DBG_PRINTF(FORMAT, ARG)
  #define DBG_PRINTF2(FORMAT, ARG1, ARG2)
#endif


//
// Global variable
//

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  //
  // Initialize the fw-box core (early stage)
  //
  fbEarlyBegin(DEVICE_TYPE, FIRMWARE_VERSION);

  //
  // Initialize the fw-box core
  //
  fbBegin(DEVICE_TYPE, FIRMWARE_VERSION);

  //
  // Setup MQTT subscribe callback
  //
  setRcvValueCallback(onReceiveValue);

  //WiFi.disconnect(); // HTM_DEBUG
} // void setup()

void loop()
{

  FwBoxIns.setValue(VAL_INDEX_LED, !digitalRead(LED_BUILTIN)); // This value is reverse

  //
  // Run the handle
  //
  fbHandle();

} // END OF "void loop()"

void onReceiveValue(int valueIndex, String* payload)
{
  DBG_PRINT("onReceiveValue valueIndex = ");
  DBG_PRINTLN(valueIndex);
  DBG_PRINT("onReceiveValue *payload = ");
  DBG_PRINTLN(*payload);

  if(valueIndex == VAL_INDEX_LED) {
    if(payload->equals("ON") == true)
    {
      digitalWrite(LED_BUILTIN, LOW); // This value is reverse
    }
    else
    {
      digitalWrite(LED_BUILTIN, HIGH); // This value is reverse
    }
  }
}
