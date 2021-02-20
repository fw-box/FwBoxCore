//
// Copyright (c) 2021 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//

#if defined(ESP32)
  //#include <Preferences.h>
  #include <esp_wifi.h>
#endif
#include "FwBoxCore.h"
#include "FwBoxLiteJson.h"


using namespace FwBox;


FwBoxCore::FwBoxCore()
{
  //Serial.println("FwBoxCore()");

  FwBoxCore::DevCfg.Uuid = "";
  FwBoxCore::DevCfg.Type = 1;

  FwBoxCore::SysCfg.SyncInterval = DEFAULT_SYSCFG_SYNC_INTERVAL;
  FwBoxCore::SysCfg.FastCmdInterval = DEFAULT_SYSCFG_FASTCMD_INTERVAL;
#if ENABLE_MQTT == 1
  FwBoxCore::SysCfg.EnMqtt = 0;
  FwBoxCore::SysCfg.MqttServer = "";
  FwBoxCore::SysCfg.MqttUser = "";
  FwBoxCore::SysCfg.MqttPass = "";
  FwBoxCore::SysCfg.MqttTopicType = 0;
  FwBoxCore::SubCallback = 0;
  FwBoxCore::SysCfg.MqttMode = MQTT_MODE_ACTIVE;
#endif // #if ENABLE_MQTT == 1
  FwBoxCore::SysCfg.CallAUrl = "";

  FwBoxCore::ServerAddress = "";
  FwBoxCore::Email = "";

  for (int i = 0; i < MAX_VALUE_COUNT; i++) {
    FwBoxCore::ValueArray[i] = 0.0;
    FwBoxCore::ValueIsSetArray[i] = 0;
    FwBoxCore::ValDesc[i] = "";
    FwBoxCore::ValUnit[i] = "";
    FwBoxCore::ValName[i] = "";
  }
  //FwBoxCore::ValOutIn = 0;
  //FwBoxCore::ValInvert = 0;
  
  FwBoxCore::ServerStatus = SERVER_STATUS_NOT_READY; // default

#if defined(LED_BUILTIN)
  FwBoxCore::GpioStatusLed = LED_BUILTIN;
#else
  FwBoxCore::GpioStatusLed = -1;
#endif

  FwBoxCore::PreviousTime = 0;
  FwBoxCore::PreviousDataUpdateTime = 0;
  FwBoxCore::PreviousGetFastCmdTime = 0;
  FwBoxCore::PreviousUserIsActiveTime = 0;

  FwBoxCore::FastCmdFwUdNum = 0;
  FwBoxCore::FastCmdUserActiveNum = 0;
  FwBoxCore::FastCmdBlkNum = 0;

  FwBoxCore::SyncDataImmediately = 0;

  FwBoxCore::FirstRun = true;

  FwBoxCore::FlagFwUdFileName = "";
  FwBoxCore::FlagFwUdAction = -1;
}

FwBoxCore::~FwBoxCore()
{
}

void FwBoxCore::begin()
{
    FwBoxCore::begin(1, "1.1");
}

//
// devType (int) - > 0
// fwVersion (String) - Example : 1.1, 1.2, 1.3...
// fwSize (String) - Example : 4M, 2M, 1M...
//
void FwBoxCore::begin(int devType, const char* fwVersion)
{
  FwBoxCore::DevCfg.Type = devType;
  if(strlen(fwVersion) < MAX_FW_VERSION_LENGTH) // The array (SysCfg.FwVersion) size is 8.
  	strcpy(&(FwBoxCore::SysCfg.FwVersion[0]), fwVersion);

#if defined(ESP8266)
  uint32_t flash_size = ESP.getFlashChipRealSize();
#else
  uint32_t flash_size = ESP.getFlashChipSize(); // for ESP32
#endif

  switch (flash_size) {
    case (1 * 1024 * 1024):
      strcpy(&(FwBoxCore::SysCfg.FwSize[0]), "1M"); // The array (SysCfg.FwSize) size is 4.
      break;
    case (2 * 1024 * 1024):
      strcpy(&(FwBoxCore::SysCfg.FwSize[0]), "2M"); // The array (SysCfg.FwSize) size is 4.
      break;
    case (4 * 1024 * 1024):
      strcpy(&(FwBoxCore::SysCfg.FwSize[0]), "4M"); // The array (SysCfg.FwSize) size is 4.
      break;
    case (8 * 1024 * 1024):
      strcpy(&(FwBoxCore::SysCfg.FwSize[0]), "8M"); // The array (SysCfg.FwSize) size is 4.
      break;
    case (16 * 1024 * 1024):
      strcpy(&(FwBoxCore::SysCfg.FwSize[0]), "16M"); // The array (SysCfg.FwSize) size is 4.
      break;
    default:
      strcpy(&(FwBoxCore::SysCfg.FwSize[0]), "4M"); // The array (SysCfg.FwSize) size is 4.
      break;
  }

  DBGMSG("FwBoxCore::SysCfg.FwSize=");
  DBGMSGLN(FwBoxCore::SysCfg.FwSize);

  if(FwBoxCore::GpioStatusLed >= 0)
    pinMode(FwBoxCore::GpioStatusLed, OUTPUT);

  //
  // Produce device's UUID.
  //
  String str_mac = WiFi.macAddress();
  str_mac.replace(":", "");
  str_mac.toLowerCase();
  if (str_mac.length() != 12) {
    DBGMSGLN("Error : MAC address format is wrong.");
  }
  String dev_uuid = str_mac.substring(0, 8) + "-";
  dev_uuid += str_mac.substring(8) + "-41c9-9260-608f4f2169b0";

  FwBoxCore::DevCfg.Uuid = dev_uuid; // Example : "cc50e360-ebde-41c9-9260-608f4f2169b0"

#if defined(ESP8266)
  if (WiFi.SSID().length() > 0) {
#else
  //
  // For ESP32
  // Read the SSID and password from NVS.
  //
  /*Preferences prefs;
  prefs.begin("FW_BOX_WIFI"); // use "WIFI" namespace
  String str_ssid = prefs.getString("SSID", String());
  //String str_pass = prefs.getString("PASS", String());
  DBGMSG("NVS WiFi SSID = ");
  DBGMSGLN(str_ssid);*/

  	/*Preferences prefs;
    prefs.clear();*/

  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);
  String str_ssid = String(reinterpret_cast<const char*>(conf.sta.ssid));
  String str_pass = String(reinterpret_cast<const char*>(conf.sta.password));
  DBGMSG("esp_wifi_get_config WiFi SSID = ");
  DBGMSGLN(str_ssid);

  if (str_ssid.length() > 0) {
#endif
    //
    // Waiting for WIFI connection
    //
    FwBoxCore::PreviousTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if ((millis() - FwBoxCore::PreviousTime) > 15 * 1000) {
        break;
      }
      DBGMSG(".");
      delay(500);
    }
    DBGMSGLN();
  }

  if (WiFi.status() == WL_CONNECTED) {

      DBGMSGLN("No server address.");

      //
      // Scan the fw-box server address.
      // If connected, get the server's response and parse it.
      //
      int result = FwBoxCore::scanServer();
#if defined(ESP32)
      if (result > 0) { // Error
        for (int i = 0; i < SERVER_LIST_COUNT; i++) {
          result = FwBoxSync::updateRootCa(&(SERVER_LIST[i]));
          if (result == 0) { // Success
            //
            // Try to scan again.
            //
            int result = FwBoxCore::scanServer();
            break;
          }
        }
      }
#endif // #if defined(ESP32)

      FwBoxCore::handleFwUd();

      FwBoxCore::PreviousTime = millis();
  }
}

void FwBoxCore::handle()
{
  //DBGMSGLN("FwBoxCore::handle():02");
  //DBGMSGLN("FwBoxCore::handle():03");

  if((WiFi.status() == WL_CONNECTED) && (ServerAddress.length() <= 0)) {
    if(((millis() - FwBoxCore::PreviousTime) > 40 * 1000) || (FwBoxCore::PreviousTime == 0)) { // Retrying every 40 seconds

      DBGMSGLN("No server address.");

      //
      // Scan fw-box server address.
      // If connected, get the server's response and parse it.
      //
      FwBoxCore::scanServer();

      FwBoxCore::PreviousTime = millis();
    }
  }
  else if((WiFi.status() == WL_CONNECTED) && (ServerAddress.length() > 0)) {
    //
    // Update sensor values to server and receive data from server.
    //
    if( ((millis() - FwBoxCore::PreviousDataUpdateTime) > FwBoxCore::SysCfg.SyncInterval) ||
        (FwBoxCore::SyncDataImmediately > 0) ||
        (FwBoxCore::FirstRun == true) ) {

      Serial.printf("Device's MAC Address is '%s'\n", WiFi.macAddress().c_str());
      if (WiFi.status() != WL_CONNECTED)
        Serial.println("WiFI disconnected.");
      //DBGMSGF("FwBoxCore::SyncDataImmediately=%d\n",FwBoxCore::SyncDataImmediately);
      //DBGMSGF("FwBoxCore::FirstRun=%d\n",FwBoxCore::FirstRun);
      String response = "";
      int result = 1; // Default error

      if (FwBoxCore::SyncDataImmediately > 0) {
        result = FwBoxCore::updateToServer(&response, FwBoxCore::SyncDataImmediately);
        FwBoxCore::SyncDataImmediately = 0;
      }
      else {
        result = FwBoxCore::updateToServer(&response, 0);
      }

      //DBGMSGLN("FwBoxCore::handle():07");
      //
      // After sending http(https) request, check the respond.
      //
      if (result == 0) { // Success
        result = FwBoxCore::parseServerResponse(&response);
        FwBoxCore::handleFwUd();
      }

      FwBoxCore::FirstRun = false;

      FwBoxCore::PreviousDataUpdateTime = millis();
      FwBoxCore::PreviousGetFastCmdTime = millis();
    }

    //DBGMSGLN("FwBoxCore::handle():08");
#if ENABLE_MQTT == 1
    FwBoxCore::handleMqtt();
#endif // #if ENABLE_MQTT == 1


    FwBoxCore::handleCallAUrl();


    if((millis() - FwBoxCore::PreviousGetFastCmdTime) > FwBoxCore::SysCfg.FastCmdInterval) {
      //DBGMSGF("FwBoxCore::SysCfg.FastCmdInterval = %d\r\n", FwBoxCore::SysCfg.FastCmdInterval);
      String response = "";
      int http_code = FwBoxSync::getFastCmd(&(DevCfg.Uuid), &response);
      if (http_code == HTTP_CODE_OK || http_code == HTTP_CODE_MOVED_PERMANENTLY) {
        DBGMSGF("getFastCmd HTTP CODE = %d\r\n", http_code);
        DBGMSGLN(response);
        FwBoxCore::parseJson(&response);
      }

      FwBoxCore::PreviousGetFastCmdTime = millis();
    } // END OF "if((millis() - FwBoxCore::PreviousGetFastCmdTime) > FwBoxCore::SysCfg.FastCmdInterval)"
  } // END OF "else if((WiFi.status() == WL_CONNECTED) && (ServerAddress.length() > 0))"
  else {
  	//
  	// If no WiFi connection, display the device's ID.
  	//
    if((millis() - FwBoxCore::PreviousDataUpdateTime) > 5000) {
      Serial.printf("Device's MAC Address is '%s'\n", WiFi.macAddress().c_str());
      FwBoxCore::PreviousDataUpdateTime = millis();
    }
  }

  //DBGMSGLN("FwBoxCore::handle() : 02");

  if((millis() - FwBoxCore::PreviousUserIsActiveTime) > TIMEOUT_USER_IS_ACTIVE) {
    //
    // User is not active, set the interval to original time.
    //
    FwBoxCore::setSyncIntervalLong();
  }

  //DBGMSGLN("FwBoxCore::handle() : 04");
}


void FwBoxCore::setGpioStatusLed(int gpioStatusLed)
{
  if(gpioStatusLed >= 0) {
    FwBoxCore::GpioStatusLed = gpioStatusLed;
    pinMode(FwBoxCore::GpioStatusLed, OUTPUT);
  }
}

DeviceConfig* FwBoxCore::getDeviceConfig()
{
  return &(FwBoxCore::DevCfg);
}

SystemConfig* FwBoxCore::getSystemConfig()
{
  return &(FwBoxCore::SysCfg);
}

void FwBoxCore::printSystemConfig()
{
  DBGMSG("SysCfg.DevName = ");
  DBGMSGLN(SysCfg.DevName);
  DBGMSG("SysCfg.SyncInterval = ");
  DBGMSGLN(SysCfg.SyncInterval);
  DBGMSG("SysCfg.FastCmdInterval = ");
  DBGMSGLN(SysCfg.FastCmdInterval);
#if ENABLE_MQTT == 1
  DBGMSG("SysCfg.EnMqtt = ");
  DBGMSGLN(SysCfg.EnMqtt);
#endif // #if ENABLE_MQTT == 1
}

String* FwBoxCore::getServerAddress()
{
  return &(FwBoxCore::ServerAddress);
}

void FwBoxCore::setEmail(const char* email)
{
  FwBoxCore::Email = email;
  //DBGMSG("Email = ");
  //DBGMSGLN(FwBoxCore::Email);
}

String FwBoxCore::getEmail()
{
  return FwBoxCore::Email;
}

String FwBoxCore::getDevName()
{
  return FwBoxCore::SysCfg.DevName;
}

String FwBoxCore::getSimpleChipId()
{
#if defined(ESP8266)
  char chip_id[24];
  memset(&(chip_id[0]), 0, 24);
  sprintf(&(chip_id[0]), "%08x", ESP.getChipId());
  String simple_chip_id = (&(chip_id[4]));
  //DBGMSG("chip_id = ");
  //DBGMSGLN(chip_id);
  //DBGMSGF("ESP.getChipId() = 0x%x\n", ESP.getChipId());
  //DBGMSG("simple_chip_id = ");
  //DBGMSGLN(simple_chip_id);
  return simple_chip_id;
#else
  uint32_t i_chip_id = 0;
  char chip_id[24];
  memset(&(chip_id[0]), 0, 24);

  for (int i=0; i<17; i=i+8) {
    i_chip_id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  sprintf(&(chip_id[0]), "%08x", i_chip_id);
  String simple_chip_id = (&(chip_id[4]));
  DBGMSG("simple_chip_id = ");
  DBGMSGLN(simple_chip_id);
  return simple_chip_id;
#endif
}

int FwBoxCore::scanServer()
{
  int http_code = 0;

  //
  // Test the server addresses.
  //
  for (int i = 0; i < SERVER_LIST_COUNT; i++) {
    DBGMSG("Connecting to server - ");
    DBGMSG(SERVER_LIST[i]);
    DBGMSGLN("...");

    //
    // Send request to server.
    //
    FwBoxSync::setServerAddress(&(SERVER_LIST[i]));
    String response = "";
    http_code = FwBoxSync::connect(DevCfg.Type, &(DevCfg.Uuid), SysCfg.FwVersion, SysCfg.FwSize, FwBoxCore::Email.c_str(), &response);

    if (http_code == HTTP_CODE_OK || http_code == HTTP_CODE_MOVED_PERMANENTLY) {
      DBGMSG("response.length()=");
      DBGMSGLN(response.length());
      DBGMSGLN(response);
      
      FwBoxCore::ServerStatus = SERVER_STATUS_OK; // Server is ok

      //
      // Check server's response.
      //
      response.trim();
      if (response.length() > 0) {
        if (FwBoxCore::parseJson(&response) == 0) { // Success
          DBGMSGLN("=== Found Server ===");
          FwBoxCore::ServerAddress = SERVER_LIST[i];

#if ENABLE_MQTT == 1
          //
          // Run after "FwBoxCore::parseJson".
          //
          if ((FwBoxCore::SysCfg.EnMqtt == 1) && (FwBoxCore::MqttClient == 0)) {
            DBGMSGLN("'Enable MQTT' is true.");
            if (FwBoxCore::SysCfg.MqttServer.length() > 0) {
              DBGMSGLN("New a 'PubSubClient' instance.");
              DBGMSG("Server : ");
              DBGMSGLN(FwBoxCore::SysCfg.MqttServer);
              FwBoxCore::MqttClient = new PubSubClient(FwBoxCore::SysCfg.MqttServer.c_str(), 1883, FwBoxCore::EspClient);
              //FwBoxCore::MqttClient->setServer(FwBoxCore::SysCfg.MqttServer.c_str(), 1883);
              if (FwBoxCore::SubCallback != 0)
              	FwBoxCore::MqttClient->setCallback(FwBoxCore::SubCallback);
            }
            else
              FwBoxCore::MqttClient = 0;
          }
#endif // #if ENABLE_MQTT == 1
        } // END OF "if(FwBoxCore::parseJson(&response) == 0)"
      } // END OF "if(response.length() > 0)"

      return 0; // Success
    }
    else {
      FwBoxCore::ServerStatus = SERVER_STATUS_NOT_OK; // Server is not ok
    }
    
  } // END OF "for (int i = 0; i < SERVER_LIST_COUNT; i++)"

  return 1; // Error
}

void FwBoxCore::setValue(int valIndex, float value)
{
  if ((valIndex >= 0) && (valIndex < MAX_VALUE_COUNT)) {
    ValueArray[valIndex] = value;
    ValueIsSetArray[valIndex] = 1;
  }
}

float FwBoxCore::getValue(int valIndex)
{
  if ((valIndex >= 0) && (valIndex < MAX_VALUE_COUNT)) {
    return ValueArray[valIndex];
  }
}

int FwBoxCore::updateToServer(String* response, int extCmd)
{
  //DBGMSGF("FwBoxCore::updateToServer extCmd = %d\n", extCmd);
  int dev_index = 0;
  FwBoxSync::clearUrl();
  FwBoxSync::setServerAddress(&(FwBoxCore::ServerAddress));
  FwBoxSync::appendDevice(dev_index, &(DevCfg.Uuid));
  for (int i = 0; i < MAX_VALUE_COUNT; i++) {
    if (FwBoxCore::ValueIsSetArray[i] == 1) {
      FwBoxSync::appendValue(dev_index, i, FwBoxCore::ValueArray[i]);
    }
  }

  //
  // Update values to server and get response.
  //
  int http_code = FwBoxSync::update(response, extCmd);
  if (http_code == HTTP_CODE_OK || http_code == HTTP_CODE_MOVED_PERMANENTLY) {
    DBGMSGF("HTTP CODE = %d\r\n", http_code);
    DBGMSGLN(*response);
    FwBoxCore::ServerStatus = SERVER_STATUS_OK; // Server is ok
  }
  else {
  	FwBoxCore::ServerStatus = SERVER_STATUS_NOT_OK; // Server is not ok
  }

  //
  // Clear value array
  //
  for (int i = 0; i < MAX_VALUE_COUNT; i++) {
    FwBoxCore::ValueIsSetArray[i] = 0;
  }

  return 0; // Success
}

int FwBoxCore::parseServerResponse(String* response)
{
  response->trim();
  //DBGMSGLN(*response);
  if (response->length() > 0) {
    int from_0 = 0;
    int to_0 = response->indexOf(">>>@", from_0);
    if (to_0 >= 0) {
      int from_1 = to_0 + 4;
      int to_1 = response->indexOf(">>>@", from_1);
      if (to_1 >= 0) {
        int from_2 = to_1 + 4;

        String str_config = response->substring(from_0, to_0);
        //String str_sen_data = response->substring(from_1, to_1);
        //String str_script = response->substring(from_2);
        //DBGMSG("str_config = ");
        //DBGMSGLN(str_config);
        //DBGMSGLN(str_sen_data);
        //DBGMSGLN(str_script);

        if (str_config.length() > 0) {
          FwBoxCore::parseJson(&str_config);
        } // END OF "if(str_config.length() > 0)"
      }
    } // END OF "if(to_0 >= 0)"
  } // END OF "if(response->length() > 0)"

  return 0; // Success
}

int FwBoxCore::parseJson(String* jsonData)
{
  FwBoxLiteJson lj;
  String temp = "";
  int temp_i = 0xfe;
  String temp_arr[MAX_VALUE_COUNT];

  if (lj.getString(jsonData, "DN", &temp) == 0) { // success
    FwBoxCore::SysCfg.DevName = temp;
    DBGMSG("[DN]FwBoxCore::SysCfg.DevName=");
    DBGMSGLN(FwBoxCore::SysCfg.DevName);
  }

  temp = "";
  if (lj.getString(jsonData, "P0", &temp) == 0) { // success
    FwBoxCore::Parameter = temp;
    DBGMSG("[P0]FwBoxCore::Parameter=");
    DBGMSGLN(FwBoxCore::Parameter);
  }

  /*temp_i = 0xfe;
  if (lj.getInt(jsonData, "VOI", &temp_i) == 0) { // success
    if(temp_i != 0xfe) {
      FwBoxCore::ValOutIn = temp_i;
      DBGMSG("[VOI]ValOutIn=");
      DBGMSGLN(FwBoxCore::ValOutIn);
    }
  }*/

  /*temp_i = 0xfe;
  if (lj.getInt(jsonData, "VI", &temp_i) == 0) { // success
    if(temp_i != 0xfe) {
      FwBoxCore::ValInvert = temp_i;
      DBGMSG("[VI]ValInvert=");
      DBGMSGLN(FwBoxCore::ValInvert);
    }
  }*/

  temp = "";
  if (lj.getString(jsonData, "VT", &temp) == 0) { // success
    DBGMSG("[VT]ValType=");
    DBGMSGLN(temp);
    
    //
    // Clear the char - "
    //
    temp.replace("\"", "");
    //DBGMSG("[VT]ValType=");
    //DBGMSGLN(temp);
  	FwBoxUtil util;
  	const char* tp = temp.c_str();
  	int pos = 0;
    util.untilNotSpace(tp, &pos);
    //DBGMSG(tp[pos]);DBGMSG("*");
    if (tp[pos] == '[') {
      pos++; // Skip '['
      //DBGMSG(tp[pos]);DBGMSG("*");
      for (int tpi = 0; tpi < MAX_VALUE_COUNT; tpi++) {
        //DBGMSG(tp[pos]);DBGMSG("*");
        int num_type = NUMBER_TYPE_ERROR;
        String token = util.untilNotNumber(tp, &pos, &num_type);
        if (num_type != NUMBER_TYPE_ERROR) {
          //
          // Transform HEX string to int
          //
          FwBoxCore::ValType[tpi] = (int)strtol(token.c_str(), 0, 16);
          //DBGMSG(num_type);
          //DBGMSG(",");
          //DBGMSG(token);
          //DBGMSG(",");
          //DBGMSG(tpi);
          //DBGMSG(",");
          //DBGMSGLN(FwBoxCore::ValType[tpi]);
                    
          util.untilNotSpace(tp, &pos);
          if (tp[pos] == ',') {
            pos++; // Skip ','
          }
          else if (tp[pos] == ']') { // reach the end.
            break;
          }
        }
      }
    }
    //FwBoxCore::Parameter = temp;
    //DBGMSG("[P0]FwBoxCore::Parameter=");
    //DBGMSGLN(FwBoxCore::Parameter);
  }

  temp_i = MAX_VALUE_COUNT;
  if (lj.getArrayString(jsonData, "VN", temp_arr, &temp_i) == 0) { // success
    FwBoxCore::ValCount = temp_i;
    DBGMSGLN("[VN]ValName=");
    for(int ii = 0; ii < temp_i; ii++) {
      temp_arr[ii].toLowerCase();
      FwBoxCore::ValName[ii] = temp_arr[ii];
      DBGMSGLN(FwBoxCore::ValName[ii]);
    }

    DBGMSG("ValCount=");
    DBGMSGLN(temp_i);
  }

  temp_i = MAX_VALUE_COUNT;
  if (lj.getArrayString(jsonData, "VD", temp_arr, &temp_i) == 0) { // success
    DBGMSGLN("[VD]ValDesc=");
    for(int ii = 0; ii < temp_i; ii++) {
      FwBoxCore::ValDesc[ii] = temp_arr[ii];
      DBGMSGLN(FwBoxCore::ValDesc[ii]);
    }

    if(temp_i > FwBoxCore::ValCount)
      FwBoxCore::ValCount = temp_i;

    DBGMSG("ValCount=");
    DBGMSGLN(temp_i);
  }

  temp_i = MAX_VALUE_COUNT;
  if (lj.getArrayString(jsonData, "VU", temp_arr, &temp_i) == 0) { // success
    DBGMSGLN("[VU]ValUnit=");
    for(int ii = 0; ii < temp_i; ii++) {
      FwBoxCore::ValUnit[ii] = temp_arr[ii];
      DBGMSGLN(FwBoxCore::ValUnit[ii]);
    }
  }

  temp_i = 0xfe;
  if (lj.getInt(jsonData, "FU", &temp_i) == 0) { // success
    if(temp_i == 1) { // Got the firmware updating command
      temp_i = 0xfe;
      if (lj.getInt(jsonData, "AT", &temp_i) == 0) { // success
        FwBoxCore::FlagFwUdAction = temp_i;
        temp = "";
        if (lj.getString(jsonData, "FL", &temp) == 0) { // success
          FwBoxCore::FlagFwUdFileName = temp;
        }
      }
    }
  }

  int server_fast_cmd_num = 0;
  if(lj.getInt(jsonData, "FCFN", &server_fast_cmd_num) == 0) { // success
    DBGMSGF("FastCmdFwUdNum=%d\n", FwBoxCore::FastCmdFwUdNum);
    if(server_fast_cmd_num > FwBoxCore::FastCmdFwUdNum) {
      FwBoxCore::SyncDataImmediately = CMD_DEV_REQUEST_FIRMWARE_INFO;
      FwBoxCore::FastCmdFwUdNum = server_fast_cmd_num;
    }
  }

  server_fast_cmd_num = 0;
  if(lj.getInt(jsonData, "FCRN", &server_fast_cmd_num) == 0) { // success
    DBGMSGF("FastCmdResetNum=%d\n", FwBoxCore::FastCmdResetNum);
    if(server_fast_cmd_num > FwBoxCore::FastCmdResetNum) {
#if defined(ESP8266)
      ESP.reset();
#else
      ESP.restart();
#endif
      while(1) {
        FwBoxCore::FastCmdResetNum = server_fast_cmd_num;
      }
    }
  }

  server_fast_cmd_num = 0;
  if(lj.getInt(jsonData, "FCUN", &server_fast_cmd_num) == 0) { // success
    DBGMSGF("FastCmdUserActiveNum=%d\n", FwBoxCore::FastCmdUserActiveNum);
    if(server_fast_cmd_num > FwBoxCore::FastCmdUserActiveNum) {
      FwBoxCore::setSyncIntervalShort();
      //DBGMSGLN("FwBoxCore::setSyncIntervalShort()...");
      FwBoxCore::FastCmdUserActiveNum = server_fast_cmd_num;
    }
  }

  server_fast_cmd_num = 0;
  if(lj.getInt(jsonData, "FCBN", &server_fast_cmd_num) == 0) { // success
  	DBGMSGF("FastCmdBlkNum=%d\n", FwBoxCore::FastCmdBlkNum);
    if(server_fast_cmd_num > FwBoxCore::FastCmdBlkNum) {
      if(FwBoxCore::GpioStatusLed >= 0) {
        int original_status = digitalRead(FwBoxCore::GpioStatusLed);
        for (int i = 0; i < 10; i++) {
          digitalWrite(FwBoxCore::GpioStatusLed, 0);
          delay(100);
          digitalWrite(FwBoxCore::GpioStatusLed, 1);
          delay(100);
        }
        digitalWrite(FwBoxCore::GpioStatusLed, original_status);
      }
      FwBoxCore::setSyncIntervalShort();
      FwBoxCore::FastCmdBlkNum = server_fast_cmd_num;
    }
  }

#if ENABLE_MQTT == 1
  temp_i = 0xfe;
  if (lj.getInt(jsonData, "EM", &temp_i) == 0) { // success
    FwBoxCore::SysCfg.EnMqtt = temp_i;
    FwBoxCore::setSyncIntervalShort();
    
    if(FwBoxCore::SysCfg.EnMqtt == 1) {
      temp = "";
      if (lj.getString(jsonData, "MS", &temp) == 0) { // success
        FwBoxCore::SysCfg.MqttServer = temp;
      }
      temp = "";
      if (lj.getString(jsonData, "MU", &temp) == 0) { // success
        FwBoxCore::SysCfg.MqttUser = temp;
      }
      temp = "";
      if (lj.getString(jsonData, "MP", &temp) == 0) { // success
        FwBoxCore::SysCfg.MqttPass = temp;
      }
      temp_i = 0xfe;
      if (lj.getInt(jsonData, "MTT", &temp_i) == 0) { // success
        FwBoxCore::SysCfg.MqttTopicType = temp_i;
      }
      temp = "";
      if(lj.getString(jsonData, "MT", &temp) == 0) { // success
        FwBoxCore::SysCfg.MqttTopic = temp;
      }
      temp = "";
      if(lj.getString(jsonData, "MPL", &temp) == 0) { // success
        FwBoxCore::SysCfg.MqttPayload = temp;
      }
    }
  }

#endif // #if ENABLE_MQTT == 1

  temp = "";
  if (lj.getString(jsonData, "CAU", &temp) == 0) { // success
    FwBoxCore::SysCfg.CallAUrl = temp;
    DBGMSG("CallAUrl=");
    DBGMSGLN(FwBoxCore::SysCfg.CallAUrl);
  }

  return 0; // Success
}

void FwBoxCore::setSyncIntervalShort()
{
  //
  // User is on line, short the interval.
  //
  FwBoxCore::SysCfg.SyncInterval = DEFAULT_SYSCFG_SYNC_INTERVAL_SHORT;
  FwBoxCore::SysCfg.FastCmdInterval = DEFAULT_SYSCFG_FASTCMD_INTERVAL_SHORT;
  FwBoxCore::PreviousUserIsActiveTime = millis(); // Update the user is active time.
}

void FwBoxCore::setSyncIntervalLong()
{
  FwBoxCore::SysCfg.SyncInterval = DEFAULT_SYSCFG_SYNC_INTERVAL;
  FwBoxCore::SysCfg.FastCmdInterval = DEFAULT_SYSCFG_FASTCMD_INTERVAL;
}

void FwBoxCore::handleFwUd()
{
  if((FwBoxCore::FlagFwUdFileName.length() > 0) && (FwBoxCore::FlagFwUdAction > 0)) {
    FwBoxSync::updateFirmwareByHttp(&(FwBoxCore::DevCfg.Uuid), FwBoxCore::FlagFwUdFileName.c_str(), FwBoxCore::SysCfg.FwSize, FwBoxCore::FlagFwUdAction, FwBoxCore::GpioStatusLed);
  }
}

#if ENABLE_MQTT == 1

void FwBoxCore::handleMqtt()
{
  static unsigned long previous_mqtt_publishing_time = 0;

  if(FwBoxCore::MqttClient != 0) {
    int temp_interval = DEFAULT_MQTT_PUBLISH_INTERVAL;

    //
    // When the MQTT publish server is ThingSpeak, increase the interval.
    //
    if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_THING_SPEAK)
        temp_interval = 20 * 1000;

    if((millis() - previous_mqtt_publishing_time) > temp_interval) {
      if(FwBoxCore::MqttClient->connected() == true) {
        //
        // Publish MQTT data for active mode.
        //
        if(FwBoxCore::SysCfg.MqttMode == MQTT_MODE_ACTIVE) {
          FwBoxCore::mqttPublish();
        }
      } // END OF "if(FwBoxCore::MqttClient->connected() == true)"
      else {
        int mqtt_conn_status = 0;
        String simple_chip_id = FwBoxCore::getSimpleChipId();
        //String mqtt_user = FwBoxCore::SysCfg.MqttUser;
        
        //
        // Use chip id as MQTT user, this is for ThingSpeak.
        //
        //if(FwBoxCore::SysCfg.MqttUser.length() <= 0)
        //    mqtt_user = simple_chip_id;
        if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_THING_SPEAK) {
          simple_chip_id = "fw_box_" + simple_chip_id;
          mqtt_conn_status = FwBoxCore::MqttClient->connect(simple_chip_id.c_str());
          DBGMSGLN(simple_chip_id);
        }
        else {
          mqtt_conn_status = FwBoxCore::MqttClient->connect(simple_chip_id.c_str(), FwBoxCore::SysCfg.MqttUser.c_str(), FwBoxCore::SysCfg.MqttPass.c_str());
          DBGMSGLN(simple_chip_id);
          DBGMSGLN(FwBoxCore::SysCfg.MqttUser);
          DBGMSGLN(FwBoxCore::SysCfg.MqttPass);
        }

        if(mqtt_conn_status) {
          DBGMSGLN("MQTT : Connected");
          
          //
          // Don't do subscribe when the server is ThingSpeak
          //
          if(FwBoxCore::SysCfg.MqttTopicType != MQTT_TOPIC_TYPE_THING_SPEAK) {
            FwBoxCore::mqttSubscribe();
          }

          //
          // Publish MQTT data for active mode.
          //
          if(FwBoxCore::SysCfg.MqttMode == MQTT_MODE_ACTIVE) {
            FwBoxCore::mqttPublish();
          }
        }
        else {
          DBGMSGLN("MQTT : Failed");
          DBGMSGLN(FwBoxCore::MqttClient->state());
        }
      }

      previous_mqtt_publishing_time = millis();
    } // END OF "if((millis() - previous_mqtt_publishing_time) > pub_interval)"

    if(FwBoxCore::MqttClient->connected() == true) {
      FwBoxCore::MqttClient->loop();
    }
  } // END OF "if(FwBoxCore::MqttClient != 0)"

}

void FwBoxCore::mqttPublish()
{
  String simple_chip_id = FwBoxCore::getSimpleChipId();

  if (FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_HOME_ASSISTANT) {
    String str_payload = "{";

    for (int vi = 0; vi < FwBoxCore::ValCount; vi++) {
      //
      // Skip if this is an input value.
      //
      if (getOutIn(FwBoxCore::ValType[vi]) == 1)
        continue;

      //
      // Skip if this is a button
      //
      //if (FwBoxCore::ValName[vi].equals("button") == true)
      if (FwBoxCore::ValType[vi] == VALUE_TYPE_OUT_BUTTON)
        continue;

      if(str_payload.length() > 1) // This is not the first value
        str_payload += ",";

      str_payload += "\"";
      str_payload += FwBoxCore::ValName[vi];
      str_payload += "\":";
      str_payload += FwBoxCore::ValueArray[vi];
    }
    
    if(str_payload.length() == 1) // no data
      return;

    str_payload += "}";

    //
    // Produce the topic string
    //
    String str_topic = "fw_box/" + simple_chip_id + "/data";

    //
    // Only publish the output values.
    //
    FwBoxCore::MqttClient->publish(str_topic.c_str(), str_payload.c_str(), true);
    DBGMSGLN(str_topic);
    DBGMSGLN(str_payload);
  }
  /*else if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_THING_SPEAK) {
    String str_payload = FwBoxCore::SysCfg.MqttPayload;
    for(int vi = 0; vi < FwBoxCore::ValCount; vi++) {
      //
      // Skip if this is an input value.
      //
      if (getOutIn(FwBoxCore::ValType[vni]) == 1) // 0 is OUT, 1 is IN
        continue;

      String key_val = "{val";
      key_val += vi;
      key_val += "}";
      str_payload.replace(key_val, String(FwBoxCore::ValueArray[vi]));
    }

    //
    // Only publish the output values.
    //
    FwBoxCore::MqttClient->publish(FwBoxCore::SysCfg.MqttTopic.c_str(), str_payload.c_str(), true);
    DBGMSGLN(FwBoxCore::SysCfg.MqttTopic);
    DBGMSGLN(str_payload);
  }*/
  else if (FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_CUSTOM) {
    String str_payload = FwBoxCore::SysCfg.MqttPayload;
    for (int vi = 0; vi < FwBoxCore::ValCount; vi++) {
      //
      // Skip if this is an input value.
      //
      if (getOutIn(FwBoxCore::ValType[vi]) == 1) // 0 is OUT, 1 is IN
        continue;

      String key_val = "{val";
      key_val += vi;
      key_val += "}";
      str_payload.replace(key_val, String(FwBoxCore::ValueArray[vi]));
    }

    //
    // Only publish the output values.
    // Topic and payload can't be empty.
    //
    if ((FwBoxCore::SysCfg.MqttTopic.length() > 0) && (str_payload.length() > 0)) {
      FwBoxCore::MqttClient->publish(FwBoxCore::SysCfg.MqttTopic.c_str(), str_payload.c_str(), true);
      DBGMSGLN(FwBoxCore::SysCfg.MqttTopic);
      DBGMSGLN(str_payload);
    }
    else {
        DBGMSGLN("Topic or payload are empty.");
    }
  }
  else {
/*
    boolean all_is_zero = true;
    for (int vni = 0; vni < FwBoxCore::ValCount; vni++) {
      if(FwBoxCore::ValueArray[vni] > 0) {
        all_is_zero = false;
        break;
      }
    }
    if(all_is_zero == false) {
      char subscribe_topic[] = "v1/devices/me/attributes"; // Fixed topic. ***DO NOT MODIFY***
      char publish_topic[]   = "v1/devices/me/telemetry";  // Fixed topic. ***DO NOT MODIFY***
      String str_json = "{";
      for (int vni = 0; vni < FwBoxCore::ValCount; vni++) {
        if(vni == 0) {
          str_json += "\"";
          str_json += FwBoxCore::ValName[vni];
          str_json += "\":\"";
          str_json += FwBoxCore::ValueArray[vni];
          str_json += "\"";
        }
        else {
          str_json += ",\"";
          str_json += FwBoxCore::ValName[vni];
          str_json += "\":\"";
          str_json += FwBoxCore::ValueArray[vni];
          str_json += "\"";
        }
      }
      str_json += "}";
      FwBoxCore::MqttClient->publish(publish_topic, str_json.c_str(), true);
      DBGMSGLN(publish_topic);
      DBGMSGLN(str_json);
    }
*/
  }
}

void FwBoxCore::mqttSubscribe()
{
  DBGMSGLN("MQTT : mqttSubscribe");
  DBGMSG("MqttTopicType:");
  DBGMSGLN(FwBoxCore::SysCfg.MqttTopicType);

  String simple_chip_id = FwBoxCore::getSimpleChipId();

  /*DBGMSGLN("=== SHOW VAL OUT IN ===");
  for (int vni = 0; vni < FwBoxCore::ValCount; vni++) {
  	DBGMSG("VAL");
  	DBGMSG(vni);
  	DBGMSG("=");
  	DBGMSGLN(getOutIn(FwBoxCore::ValType[vni]));
  }*/

  if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_HOME_ASSISTANT) {
    for (int vni = 0; vni < FwBoxCore::ValCount; vni++) {
      //
      // Check the value is input or output.
      //
      //if(!(FwBoxCore::ValOutIn & (1 << vni))) {
      if (getOutIn(FwBoxCore::ValType[vni]) == 0) { // 0 is OUT, 1 is IN
          continue; // It's an output value, skip it.
      }

      //
      // Only subscribe input values.
      //
      String str_topic = "fw_box/" + simple_chip_id + "/" + FwBoxCore::ValName[vni] + "/set";
      FwBoxCore::MqttClient->subscribe(str_topic.c_str());
      DBGMSGLN(str_topic);
    } // END OF "for (int vni = 0; vni < FwBoxCore::ValCount; vni++)"
  }
  else if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_CUSTOM) {
    for (int vni = 0; vni < FwBoxCore::ValCount; vni++) {
      //
      // Check the value is input or output.
      //
      //if(!(FwBoxCore::ValOutIn & (1 << vni))) {
      if (getOutIn(FwBoxCore::ValType[vni]) == 0) { // 0 is OUT, 1 is IN
          continue; // It's an output value, skip it.
      }

      //
      // Only subscribe input values.
      //
      String str_topic = "fw_box/" + simple_chip_id + "/" + FwBoxCore::ValName[vni] + "/set";
      FwBoxCore::MqttClient->subscribe(str_topic.c_str());
      DBGMSGLN(str_topic);
    } // END OF "for (int vni = 0; vni < FwBoxCore::ValCount; vni++)"
  }
}

void FwBoxCore::setMqttCallback(MqttCallback callback)
{
    FwBoxCore::SubCallback = callback;
}

void FwBoxCore::mqttPublish(uint8_t valIndex, const char* payload)
{
  if(valIndex == 0xff)
    return;

  String simple_chip_id = FwBoxCore::getSimpleChipId();

  if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_HOME_ASSISTANT) {
    //
    // Produce the topic string
    //
    String str_topic = "fw_box/" + simple_chip_id + "/" + FwBoxCore::ValName[valIndex];
    
    //
    // Publish it
    //
    FwBoxCore::MqttClient->publish(str_topic.c_str(), payload, true);
    DBGMSGLN(str_topic);
    DBGMSGLN(payload);
  }
  else if(FwBoxCore::SysCfg.MqttTopicType == MQTT_TOPIC_TYPE_IDEAS_CHAIN) {

  }
}

void FwBoxCore::setMqttMode(uint8_t mqttMode)
{
  FwBoxCore::SysCfg.MqttMode = mqttMode;
}

#endif // #if ENABLE_MQTT == 1

void FwBoxCore::handleCallAUrl()
{
  static unsigned long previous_call_a_url_time = 0;
  
  if ((millis() - previous_call_a_url_time) > DEFAULT_CALL_A_URL_INTERVAL) {

    String str_url = FwBoxCore::SysCfg.CallAUrl;
    for(int vi = 0; vi < FwBoxCore::ValCount; vi++) {
      //
      // Skip if this is an input value.
      //
      //if(FwBoxCore::ValOutIn & (1 << vi))
      if (getOutIn(FwBoxCore::ValType[vi]) == 1) // 0 is OUT, 1 is IN
        continue;

      String key_val = "{val";
      key_val += vi;
      key_val += "}";
      str_url.replace(key_val, String(FwBoxCore::ValueArray[vi]));
    }
    
    DBGMSGLN(str_url);
    String respond = "";
    FwBoxSync::sendHttpGet(str_url.c_str(), &respond);
    //DBGMSGLN(respond);
    
    previous_call_a_url_time = millis();

  }
}

void FwBoxCore::setValDesc(int valIndex, const char* desc)
{
  if((valIndex >= 0) && (valIndex < FwBoxCore::ValCount))
    FwBoxCore::ValDesc[valIndex] = desc;
}

void FwBoxCore::setValUnit(int valIndex, const char* unit)
{
  if((valIndex >= 0) && (valIndex < FwBoxCore::ValCount))
    FwBoxCore::ValUnit[valIndex] = unit;
}

String FwBoxCore::getValDesc(int index)
{
    if((index >= 0) && (index < FwBoxCore::ValCount))
        return FwBoxCore::ValDesc[index];
    else
        return "";
}

String FwBoxCore::getValUnit(int index)
{
    if((index >= 0) && (index < FwBoxCore::ValCount))
        return FwBoxCore::ValUnit[index];
    else
        return "";
}

String FwBoxCore::getValName(int index)
{
    if((index >= 0) && (index < FwBoxCore::ValCount))
        return FwBoxCore::ValName[index];
    else
        return "";
}

uint8_t FwBoxCore::getValCount()
{
    return FwBoxCore::ValCount;
}

//
// Return :
// 0 is OUT, 1 is IN
//
uint8_t FwBoxCore::getValOutIn(int index)
{
    return getOutIn(FwBoxCore::ValType[index]);
}

int FwBoxCore::getParameterArray(String* out, int length)
{
    const char* pstr = FwBoxCore::Parameter.c_str();
    int str_len = FwBoxCore::Parameter.length();
    int out_index = 0;
    int pos = -1;
    
    Serial.print("str_len=");
    Serial.println(str_len);
    
    for(int si = 0; si < str_len; si++) {
        if(pstr[si] == '\"') {
            if(pos == -1) { // Begin
                pos = si;
            }
            else { // End
                pos = -1;
                out_index++;
                continue;
            }
        }
        else {
            if(pos >= 0) {
                (out[out_index]) += pstr[si];
            }
        }
    }
    
    return out_index;
}

signed char FwBoxCore::getServerStatus()
{
	return FwBoxCore::ServerStatus;
}
