//
// Copyright (c) 2021 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//

#ifndef __FWBOXCORE__
#define __FWBOXCORE__


#include <Arduino.h>

#include "BuildSettings.h"
#include "FwBoxUtil.h"

#define SERVER_STATUS_NOT_READY -1
#define SERVER_STATUS_OK 0
#define SERVER_STATUS_NOT_OK 1

#define MAX_FW_VERSION_LENGTH 8
#define MAX_FW_SIZE_LENGTH 4

#if ENABLE_MQTT == 1

  #include <PubSubClient.h>

  typedef void (*MqttCallback)(char* topic, byte* payload, unsigned int length);

#endif // #if ENABLE_MQTT == 1

#include "FwBoxBasic.h"
#include "FwBoxSync.h"

#if ENABLE_CONFIG_SERVER == 1
  #include "FwBoxCfgServer.h"
#endif // #if ENABLE_CONFIG_SERVER == 1


#if ENABLE_MQTT == 1
  #define MQTT_MODE_ACTIVE 1
  #define MQTT_MODE_PASSIVE 2
#endif // #if ENABLE_MQTT == 1

//
// define('VALUE_TYPE', array(
//     "0x005" => "Sensor(Out)", // Temperature, Humidity or PM2.5...
//     "0x006" => "Button(Out)",
//     "0x007" => "String(Out)",
//     "0x105" => "Brightness(In)", // Brightness or PWM...
//     "0x106" => "OnOff(In)", // Relay, LED or Mosfet...
//     "0x107" => "String(In)"
// ));
//
#define getOutIn(a) ((a&0x100)>>8)

#define VALUE_TYPE_OUT_SENSOR 0x005
#define VALUE_TYPE_OUT_BUTTON 0x006
#define VALUE_TYPE_OUT_STRING 0x007
#define VALUE_TYPE_OUT_SWITCH 0x106
#define VALUE_TYPE_OUT_BRIGHTNESS 0x105
namespace FwBox {
  
  struct DeviceConfig {
    String Uuid;
    int Type;
  };
  
  //
  // Struct define for config
  //
  struct SystemConfig {
    char FwVersion[MAX_FW_VERSION_LENGTH];
    char FwSize[MAX_FW_SIZE_LENGTH];
    String DevName;
    int SyncInterval; // MilliSeconds = 1/1000 Seconds
    int FastCmdInterval; // MilliSeconds = 1/1000 Seconds
#if ENABLE_MQTT == 1
    uint8_t EnMqtt;
    String MqttServer;
    String MqttUser;
    String MqttPass;
    int MqttTopicType;
    String MqttTopic;
    String MqttPayload;
    uint8_t MqttMode;
#endif // #if ENABLE_MQTT == 1
    String CallAUrl;
  };
  
  //
  // Server address
  //
  const int SERVER_LIST_COUNT = 2;
  const String SERVER_LIST[SERVER_LIST_COUNT] = {"fw-box.com", "fw-box.com"};

  class FwBoxCore : public FwBoxBasic, public FwBoxUtil, public FwBoxSync {
    protected:
      DeviceConfig DevCfg;
      SystemConfig SysCfg;
      int GpioStatusLed;
      String ServerAddress;
      String Email;
      float ValueArray[MAX_VALUE_COUNT];
      uint8_t ValueIsSetArray[MAX_VALUE_COUNT];
      String ValDesc[MAX_VALUE_COUNT];
      String ValUnit[MAX_VALUE_COUNT];
      String ValName[MAX_VALUE_COUNT];
      bool FirstTimeHomeAssistantPublish = true;

      //
      // define('VALUE_TYPE', array(
      //     "0x005" => "Sensor(Out)",
      //     "0x006" => "Button(Out)",
      //     "0x007" => "String(Out)",
      //     "0x105" => "Brightness(In)",
      //     "0x106" => "OnOff(In)",
      //     "0x107" => "String(In)"
      // ));
      //
      uint16_t ValType[MAX_VALUE_COUNT];

      uint8_t ValCount = 0;
      String Parameter = "";
      
      //
      // -1 : Not ready
      //  0 : OK
      //  1 : Not OK
      //
      signed char ServerStatus;

      unsigned long PreviousTime;
      unsigned long PreviousDataUpdateTime;
      unsigned long PreviousGetFastCmdTime;
      unsigned long PreviousUserIsActiveTime;

      uint8_t FastCmdFwUdNum = 0;
      uint8_t FastCmdResetNum = 0;
      int FastCmdUserActiveNum = 0;
      uint16_t FastCmdBlkNum = 0;

      int SyncDataImmediately;
      boolean FirstRun = true;
      String FlagFwUdFileName;
      int FlagFwUdAction;

#if ENABLE_MQTT == 1
      WiFiClient EspClient;
      PubSubClient* MqttClient = 0;
      MqttCallback SubCallback = 0;
#endif // #if ENABLE_MQTT == 1


      int scanServer();
      int updateToServer(String* response, int extCmd);
      int parseServerResponse(String* response);
      int parseJson(String* jsonData);
      void setSyncIntervalShort();
      void setSyncIntervalLong();
      void handleFwUd();
#if ENABLE_MQTT == 1
      void handleMqtt();
      void mqttSubscribe();
#endif // #if ENABLE_MQTT == 1
      void handleCallAUrl();

    public:
      FwBoxCore();
      ~FwBoxCore();

      void begin();

      //
      // devType (int) - > 0
      // fwVersion (String) - Example : 1.1, 1.2, 1.3...
      //
      void begin(int devType, const char* fwVersion);
      void handle();

      void setGpioStatusLed(int gpioStatusLed);

      String getSimpleChipId();

      DeviceConfig* getDeviceConfig();
      SystemConfig* getSystemConfig();
      void printSystemConfig();
      String* getServerAddress();
      void setEmail(const char* email);
      String getEmail();
      String getDevName();

      void setValue(int valIndex, float value);
      float getValue(int valIndex);
      
      void setValDesc(int valIndex, const char* desc);
      void setValUnit(int valIndex, const char* unit);
      void setValUnit(int valIndex, String unit);
      String getValDesc(int index);
      String getValUnit(int index);
      String getValName(int index);
      uint16_t getValType(int index);
      uint8_t getValCount();
      uint8_t getValOutIn(int index);
      int getParameterArray(String* out, int length);
      
      signed char getServerStatus();

#if ENABLE_MQTT == 1
      void setMqttCallback(MqttCallback callback);
      void mqttPublish();
      void mqttPublish(uint8_t valIndex, const char* payload);
      void setMqttMode(uint8_t mqttMode);
#endif // #if ENABLE_MQTT == 1
  };

};

using namespace FwBox;

#endif // __FWBOXCORE__
