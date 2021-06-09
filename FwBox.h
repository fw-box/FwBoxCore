//
// Copyright (c) 2021 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//

#ifndef __FWBOX__
#define __FWBOX__

#include "BuildSettings.h"
#if defined(ESP32)
  //#include <Preferences.h>
  #include <esp_wifi.h>
#endif
#include "FwBoxCore.h"
#include "FwBoxCfgServer.h"

using namespace FwBox;

FwBoxCore FwBoxIns; // FwBoxCore instance

bool FlagFbEarlyBegin = false;
bool FlagCfgServerBegin = false;

#if ENABLE_MQTT == 1

typedef void (*ReceiveValueCallback)(int valueIndex, String* payload);

ReceiveValueCallback RcvValueCallback = 0;

void setRcvValueCallback(ReceiveValueCallback rcvValueCallback)
{
  RcvValueCallback = rcvValueCallback;
}

void mqttSubCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String str_callback_topic = String((char*)topic);
  String component;///Home assistant component

  String str_topic_head = "homeassistant/";

  for(int vi = 0; vi < FwBoxIns.getValCount(); vi++) {
    //
    // If the value is not input value, skip.
    //
    if(FwBoxIns.getValOutIn(vi) == 0) { // 0 is OUT, 1 is IN
      continue;
    }
  
    if(FwBoxIns.getValType(vi) == VALUE_TYPE_OUT_SWITCH ) {
        component = "switch";
    }
    else if(FwBoxIns.getValType(vi) == VALUE_TYPE_OUT_BRIGHTNESS ) {
        component = "light";
    }
    //
    // Consist the topic string.
    //
    String str_topic = str_topic_head + component + "/" + FwBoxIns.getSimpleChipId() + "/" + FwBoxIns.getValName(vi) + "/set";
    DBGMSGLN( str_topic);
    //
    // MQTT server send a topic to modify GPIO value.
    //
    if(str_callback_topic.equals(str_topic) == true)
    {
      String str_payload = String((char*)payload);
      if(RcvValueCallback != 0)
        RcvValueCallback(vi, &str_payload);
    }
  }
}

#endif // #if ENABLE_MQTT == 1

void fbEarlyBegin(int devType, const char* fwVersion)
{
  FlagFbEarlyBegin = true;

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

#if defined(ESP8266)
  WiFi.begin();
#else
  //
  // For ESP32
  // Read the SSID and password from NVS.
  //
  /*Preferences prefs;
  prefs.begin("FW_BOX_WIFI"); // use "WIFI" namespace
  String str_ssid = prefs.getString("SSID", String());
  String str_pass = prefs.getString("PASS", String());
  DBGMSG("fbEarlyBegin NVS WiFi SSID = ");
  DBGMSGLN(str_ssid);
  DBGMSG("fbEarlyBegin WiFi.SSID().length() = ");
  DBGMSGLN(WiFi.SSID().length());*/

  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);
  String str_ssid = String(reinterpret_cast<const char*>(conf.sta.ssid));
  String str_pass = String(reinterpret_cast<const char*>(conf.sta.password));
  DBGMSG("esp_wifi_get_config WiFi SSID = ");
  DBGMSGLN(str_ssid);
  DBGMSG("esp_wifi_get_config WiFi PASSWORD = ");
  DBGMSGLN(str_pass);

  if (str_ssid.length() > 0) {
    WiFi.begin(str_ssid.c_str(), str_pass.c_str());
    DBGMSGLN("WiFi.begin(str_ssid.c_str(), str_pass.c_str());");
  }
  else {
    WiFi.begin();
    DBGMSGLN("WiFi.begin();");
  }
#endif
}

//
// Run at setup
//
void fbBegin(int devType, const char* fwVersion)
{
  //
  // If 'fbEarlyBegin' didn't run yet, run it.
  //
  if (FlagFbEarlyBegin == false) {
    fbEarlyBegin(devType, fwVersion);
  }

#if ENABLE_MQTT == 1
  //
  // Set the MQTT callback, it must be called before "FwBoxIns.begin".
  //
  FwBoxIns.setMqttCallback(mqttSubCallback);
#endif // #if ENABLE_MQTT == 1

  //
  // Begin the FwBoxCore
  //
  FwBoxIns.begin(devType, fwVersion);


  DBGMSGF2("WiFi SSID, psk = %s, %s\n", WiFi.SSID().c_str(), WiFi.psk().c_str());
  
  
  //
  // If the SSID is not set yet, open a web server for setting.
  //
#if defined(ESP8266)
  if(WiFi.SSID().length() <= 0) {
#else
  //
  // For ESP32
  //
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);
  String str_ssid = String(reinterpret_cast<const char*>(conf.sta.ssid));
  //String str_pass = String(reinterpret_cast<const char*>(conf.sta.password));
  DBGMSG("esp_wifi_get_config WiFi SSID = ");
  DBGMSGLN(str_ssid);
  //DBGMSG("esp_wifi_get_config WiFi PASSWORD = ");
  //DBGMSGLN(str_pass);
  if (str_ssid.length() <= 0) {
#endif
    //
    // Produce the AP's SSID.
    //
    String temp_id = COMPANY;
    String str_mac = WiFi.macAddress();
    str_mac.replace(":", "");
    str_mac.toLowerCase();
    if (str_mac.length() >= 12) {
      temp_id = temp_id + "_" + str_mac.substring(8);
    }

    //
    // Set the WIFI mode to AP and STA.
    //
    DBGMSGLN("Set the WIFI mode to AP and STA.");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(temp_id.c_str(), ""); // Example of AP's SSID : "FW-BOX_ebde"

    //
    // Begin a http server for WIFI config
    //
    cfgServerBegin();
    FlagCfgServerBegin = true;
  }
}

//
// Run at loop
//
void fbHandle()
{
  FwBoxIns.handle();

  //
  // If the web server is begined, handle it.
  //
  if(FlagCfgServerBegin == true) {
    cfgServerHandle();
  }
}

#endif // __FWBOX__
