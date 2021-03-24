//
// Copyright (c) 2021 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//

#ifndef __FWBOXSYNC__
#define __FWBOXSYNC__


#include <Arduino.h>
#include "BuildSettings.h"
#include "FwBoxBasic.h"

#define MAX_URL_LENGTH 256
#define MAX_URL_LENGTH_DOUBLE 512

//
// Wifi library
//
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266httpUpdate.h>
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <WiFiClientSecure.h>
  #include <HTTPUpdate.h>
#endif

#include <PubSubClient.h>
typedef void (*FbMqttCallback)(char* topic, byte* payload, unsigned int length);

//#include <WiFiClientSecureBearSSL.h> // HTTPS


namespace FwBox {

    class FwBoxSync : public FwBoxBasic {
      protected:
        String ServerAddress = "";
        String UrlForAppending = "";
        boolean IsHttpsServer;
        char url[MAX_URL_LENGTH_DOUBLE];
        char buff[16];

        String encodeHash(const char* str);

        WiFiClient FbWiFiClient;
        PubSubClient* FbClient = 0;
        FbMqttCallback FbCallback = 0;

      public:
        FwBoxSync();
        ~FwBoxSync();
        
        int connectFb(int deviceType, String* deviceUuid, const char* fwVersion, const char* fwSize, const char* email, String* responseData);
		void setFbCallback();

        int sendHttpGet(const char* strUrl, String* payload);
        int sendHttpsGet(const char* strUrl, String* payload);

        void setServerAddress(const String* _ServerAddress);

        void clearUrl();
        String getUrl();
        
        int connect(int deviceType, String* deviceUuid, const char* fwVersion, const char* fwSize, const char* email, String* responseData);

        void appendDevice(int DeviceIndex, String* DeviceUuid);
        void appendValue(int DeviceIndex, int ValueIndex, float Value);
        int update(String* ResponseData, int extCmd);
        int getFastCmd(String* devUuid, String* responseData);
        int sendDevCmdFwUdStage(String* devUuid, int stage, String* msg, String* responseData);

#if defined(ESP32)
        int updateRootCa(const String* serverAddress);
#endif

        int updateFirmwareByHttp(String* devUuid, const char* fileName, const char* fwSize, int action, int gpioStatusLed);
		String encodeUrl(String* str);
    };
  
};

#endif // __FWBOXSYNC__
