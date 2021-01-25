
#include "FwBoxSync.h"
#include "FwBoxUtil.h"
#if defined(ESP32)
  #include <Preferences.h>
  #include "FwBoxLiteJson.h"
#endif


using namespace FwBox; 


FwBoxSync::FwBoxSync()
{
  FwBoxSync::ServerAddress = "";
  FwBoxSync::UrlForAppending = "";
  FwBoxSync::IsHttpsServer = false;
}

FwBoxSync::~FwBoxSync()
{
}

void FwBoxSync::setServerAddress(const String* serverAddress)
{
  FwBoxSync::ServerAddress = (*serverAddress);

  if((FwBoxSync::ServerAddress.equals("conplug.com.tw") == true) ||
     (FwBoxSync::ServerAddress.equals("fw-box.com") == true) ){

    FwBoxSync::IsHttpsServer = true;
  }
  else {
    FwBoxSync::IsHttpsServer = false;
  }
}

void FwBoxSync::clearUrl()
{
  FwBoxSync::UrlForAppending = "";
}

String FwBoxSync::getUrl()
{
  return FwBoxSync::UrlForAppending;
}

void FwBoxSync::appendDevice(int DeviceIndex, String* DeviceUuid)
{
  String str_temp = "&u";
  str_temp += DeviceIndex;
  str_temp += "=";
  str_temp += (*DeviceUuid);
  (FwBoxSync::UrlForAppending) += str_temp;
}

void FwBoxSync::appendValue(int DeviceIndex, int ValueIndex, float Value)
{
  if((DeviceIndex < 0) || (ValueIndex < 0))
    return;

  String str_temp = "&u";
  str_temp += DeviceIndex;
  str_temp += "v";
  str_temp += ValueIndex;
  str_temp += "=";
  str_temp += String(Value, 3); // 3 digits below the decimal point

  (FwBoxSync::UrlForAppending) += str_temp;
}

//
// Connect to the server.
//
int FwBoxSync::connect(int deviceType, String* deviceUuid, const char* fwVersion, const char* fwSize, const char* email, String* responseData)
{
  FwBoxUtil fb_util;

  memset(&(FwBoxSync::url[0]), 0, MAX_URL_LENGTH_DOUBLE);
  memset(&(FwBoxSync::buff[0]), 0, 16);

  if(FwBoxSync::IsHttpsServer == true) {
    strcpy(FwBoxSync::url, "https://");
    strcat(FwBoxSync::url, FwBoxSync::ServerAddress.c_str());
  }
  else {
    strcpy(FwBoxSync::url, "http://");
    strcat(FwBoxSync::url, FwBoxSync::ServerAddress.c_str());
  }

  strcat(FwBoxSync::url, URL_PATH_CONN);

  strcat(FwBoxSync::url, "?local_ip=");
  strcat(FwBoxSync::url, WiFi.localIP().toString().c_str());
  DBGMSGLN("connect:05");
  strcat(FwBoxSync::url, "&hs=");
  strcat(FwBoxSync::url, FwBoxSync::encodeHash(WiFi.psk().c_str()).c_str());
  DBGMSGLN("connect:06");
  strcat(FwBoxSync::url, "&dev_uuid=");
  strcat(FwBoxSync::url, deviceUuid->c_str());
  strcat(FwBoxSync::url, "&dev_type=");
  sprintf(FwBoxSync::buff, "%d", deviceType);
  strcat(FwBoxSync::url, FwBoxSync::buff);
  strcat(FwBoxSync::url, "&fw_ver=");
  strcat(FwBoxSync::url, fwVersion);
  strcat(FwBoxSync::url, "&fw_size=");
  strcat(FwBoxSync::url, fwSize);
  strcat(FwBoxSync::url, "&fw_build_dt=");
  strcat(FwBoxSync::url, fb_util.getBuildDateTime().c_str());
#if defined(ESP32)
  strcat(FwBoxSync::url, "&mcu=ESP32");
#else
  strcat(FwBoxSync::url, "&mcu=ESP8266");
#endif
  if(strlen(email) > 0) {
    strcat(FwBoxSync::url, "&email=");
    strcat(FwBoxSync::url, email);
  }
  //DBGMSGLN(FwBoxSync::url);

  return FwBoxSync::sendHttpGet(FwBoxSync::url, responseData);
}

int FwBoxSync::update(String* ResponseData, int extCmd)
{
  memset(&(FwBoxSync::url[0]), 0, MAX_URL_LENGTH_DOUBLE);
  memset(&(FwBoxSync::buff[0]), 0, 16);

  if(FwBoxSync::IsHttpsServer == true) {
    strcpy(FwBoxSync::url, "https://");
    strcat(FwBoxSync::url, FwBoxSync::ServerAddress.c_str());
  }
  else {
    strcpy(FwBoxSync::url, "http://");
    strcat(FwBoxSync::url, FwBoxSync::ServerAddress.c_str());
  }

  strcat(FwBoxSync::url, URL_PATH_SYNC_DATA);

  //strcat(FwBoxSync::url, "?wifi_pass=");
  //strcat(FwBoxSync::url, WiFi.psk().c_str());
  strcat(FwBoxSync::url, "?hs=");
  strcat(FwBoxSync::url, FwBoxSync::encodeHash(WiFi.psk().c_str()).c_str());

  //DBGMSGF("FwBoxSync::update extCmd = %d\n", extCmd);
  if(extCmd > 0) {
    strcat(FwBoxSync::url, "&cmd=");
    sprintf(FwBoxSync::buff, "%d", extCmd);
    strcat(FwBoxSync::url, FwBoxSync::buff);
  }
  strcat(FwBoxSync::url, FwBoxSync::UrlForAppending.c_str());

  return FwBoxSync::sendHttpGet(FwBoxSync::url, ResponseData);
}

//
// Fast command is sent from server to this device.
//
int FwBoxSync::getFastCmd(String* devUuid, String* responseData)
{
  String tmp_dev_uuid = (*devUuid);
  
  memset(&(FwBoxSync::url[0]), 0, MAX_URL_LENGTH_DOUBLE);

  tmp_dev_uuid.toLowerCase();

  //
  // Use "http" to do FastCmd. "http" is faster than "https".
  //
  strcpy(FwBoxSync::url, "http://");
  strcat(FwBoxSync::url, FwBoxSync::ServerAddress.c_str());

  strcat(FwBoxSync::url, URL_PATH_FAST_CMD);

  strcat(FwBoxSync::url, "/");
  strcat(FwBoxSync::url, tmp_dev_uuid.c_str());
  strcat(FwBoxSync::url, ".json");
  
  return FwBoxSync::sendHttpGet(FwBoxSync::url, responseData);
}

int FwBoxSync::sendDevCmdFwUdStage(String* devUuid, int stage, String* msg, String* responseData)
{
  char url2[MAX_URL_LENGTH_DOUBLE];
  char buff2[16];

  memset(&(url2[0]), 0, MAX_URL_LENGTH_DOUBLE);
  memset(&(buff2[0]), 0, 16);

  if(FwBoxSync::IsHttpsServer == true) {
    strcpy(url2, "https://");
    strcat(url2, FwBoxSync::ServerAddress.c_str());
  }
  else {
    strcpy(url2, "http://");
    strcat(url2, FwBoxSync::ServerAddress.c_str());
  }

  strcat(url2, URL_PATH_DEV_CMD);
  
  sprintf(buff2, "?cmd=%d", CMD_DEV_FW_UD_STAGE);
  strcat(url2, buff2);

  if(stage > 0) {
    sprintf(buff2, "&stage=%d", stage);
    strcat(url2, buff2);
  }
  else {
    return 1; // Error
  }

  if(msg->length() > 0) {
  	String str_encoded = FwBoxSync::encodeUrl(msg);
    strcat(url2, "&msg=");
    strcat(url2, str_encoded.c_str());
  }

  strcat(url2, "&dev_uuid=");
  strcat(url2, devUuid->c_str());

  //strcat(url2, "&wifi_pass=");
  //strcat(url2, WiFi.psk().c_str());
  strcat(url2, "&hs=");
  strcat(url2, FwBoxSync::encodeHash(WiFi.psk().c_str()).c_str());

  return FwBoxSync::sendHttpGet(url2, responseData);
}

int FwBoxSync::updateFirmwareByHttp(String* devUuid, const char* fileName, const char* fwSize, int action, int gpioStatusLed)
{
  String file_name = fileName;
  int fw_size_strlen = strlen(fwSize);

  memset(&(FwBoxSync::url[0]), 0, MAX_URL_LENGTH_DOUBLE);

  strcpy(FwBoxSync::url, "http://");

  strcat(FwBoxSync::url, FwBoxSync::ServerAddress.c_str());

  DBGMSG("action=");
  DBGMSGLN(action);
  if(action == FIRMWARE_UPDATE_FORCE) {
    strcat(FwBoxSync::url, URL_PATH_UPLOAD);
  }
  else if(action == FIRMWARE_UPDATE_OFFICIAL_VERSION) {
    strcat(FwBoxSync::url, URL_PATH_FIRMWARE);
  }
  else
    return 1; // Error

  if(fw_size_strlen > 0) {
    file_name.replace("{fw_size}", fwSize);
  }

  file_name.toLowerCase();

  strcat(FwBoxSync::url, "/");
  strcat(FwBoxSync::url, file_name.c_str());

  // The line below is optional. It can be used to blink the LED on the board during flashing
  // The LED will be on during download of one buffer of data from the network. The LED will
  // be off during writing that buffer to flash
  // On a good connection the LED should flash regularly. On a bad connection the LED will be
  // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
  // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
  if (gpioStatusLed >= 0) {
#if defined(ESP8266)
    ESPhttpUpdate.setLedPin(gpioStatusLed, LOW);
#else
    httpUpdate.setLedPin(gpioStatusLed, LOW);
#endif
  }

  String response = "";
  String tmp_msg = "";
  FwBoxSync::sendDevCmdFwUdStage(devUuid, FW_UD_STAGE_DEVICE_STARTED_UPDATING, &tmp_msg, &response);
  DBGMSGLN(response);

  t_httpUpdate_return ret;
  DBGMSGLN(FwBoxSync::url);
  DBGMSGLN("Updating...");
  WiFiClient client;
  
#if defined(ESP8266)
    ret = ESPhttpUpdate.update(client, FwBoxSync::url);
#else
    ret = httpUpdate.update(client, FwBoxSync::url);
#endif

  switch (ret) {
    case HTTP_UPDATE_FAILED:
#if defined(ESP8266)
      tmp_msg =  "HTTP_UPDATE_FAILD (";
      tmp_msg += ESPhttpUpdate.getLastError();
      tmp_msg += ") : ";
      tmp_msg += ESPhttpUpdate.getLastErrorString();
#else
      tmp_msg =  "HTTP_UPDATE_FAILD (";
      tmp_msg += httpUpdate.getLastError();
      tmp_msg += ") : ";
      tmp_msg += httpUpdate.getLastErrorString();
#endif
      DBGMSGLN(tmp_msg);
      response = "";
      FwBoxSync::sendDevCmdFwUdStage(devUuid, FW_UD_STAGE_ERROR, &tmp_msg, &response);
      //Serial.printf("HTM HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      tmp_msg = "HTTP_UPDATE_NO_UPDATES";
      DBGMSGLN(tmp_msg);
      response = "";
      FwBoxSync::sendDevCmdFwUdStage(devUuid, FW_UD_STAGE_ERROR, &tmp_msg, &response);
      //Serial.println("HTM HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      DBGMSGLN("HTM HTTP_UPDATE_OK");
      break;
  }

  return 0; // Success
}

#if defined(ESP32)
//
// Return -
//   0 - Success
//   1 - Expired date doesn't changed, didn't update.
//   2 - Can't connect to the server.
//
int FwBoxSync::updateRootCa(const String* serverAddress)
{
  char url2[MAX_URL_LENGTH_DOUBLE];
  Preferences prefs;
  prefs.begin("FW_BOX"); // use "FW_BOX" namespace

  memset(&(url2[0]), 0, MAX_URL_LENGTH_DOUBLE);
  String curr_expired = prefs.getString("Expired", String());

  strcpy(url2, "http://");
  strcat(url2, serverAddress->c_str());

  strcat(url2, URL_PATH_ROOT_CA);

  String res = "";
  int http_code = FwBoxSync::sendHttpGet(url2, &res);

  if (http_code == HTTP_CODE_OK || http_code == HTTP_CODE_MOVED_PERMANENTLY) {
    FwBoxLiteJson lj;

    String temp_expired = "";
    if (lj.getString(&res, "Expired", &temp_expired) == 0) { // success
      DBGMSGLN(temp_expired);
      //
      // Check the expired date.
      //
      if (curr_expired.equals(temp_expired) == true) {
        DBGMSGLN("curr_expired.equals(temp_expired)");
        return 1; // Expired date doesn't changed, didn't update.
      }
      else {
        //
        // Changed
        //
        String temp_root_ca = "";
        if (lj.getString(&res, "RootCa", &temp_root_ca) == 0) { // success
          DBGMSGLN(temp_root_ca);
          //
          // Save the RootCa to NVS.
          //
          prefs.putString("Expired", temp_expired.c_str());
          prefs.putString("RootCa", temp_root_ca.c_str());
          return 0; // Success
        }
      }
    }
  }
  DBGMSG("http_code = ");
  DBGMSGLN(http_code);

  return 2; // Can't connect to the server.
}
#endif // #if defined(ESP32)

int FwBoxSync::sendHttpGet(const char* strUrl, String* payload) {
  String temp_url = strUrl;
  temp_url.toLowerCase();
  if(temp_url.indexOf("https://") == 0) {
    return sendHttpsGet(strUrl, payload);
  }

  int httpCode = 0x7fff;
  if ((WiFi.status() == WL_CONNECTED)) {
#if defined(ESP8266)
    WiFiClient client;
    HTTPClient http;

    DBGMSGLN(strUrl);

    //DBGMSG("[HTTP] begin...\n");

    if (http.begin(client, strUrl)) {  // HTTP
      //DBGMSG("[HTTP] GET...\n");
      
      http.setTimeout(6 * 1000);

      // start connection and send HTTP header
      httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        DBGMSG("[HTTP] GET... code: ");
        DBGMSGLN(httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          (*payload) = http.getString();
          payload->trim();
          const char* cc = payload->c_str();
          if((cc[0] == 0xef) && (cc[1] == 0xbb) && (cc[2] == 0xbf)) { // UTF-8 sign
            (*payload) = payload->substring(3); // Skip UTF-8 sign
          }
        }
      }
      else {
        DBGMSG("[HTTP] GET... failed, error: ");
        DBGMSGLN(http.errorToString(httpCode).c_str());
      }

      http.end();

    }
    else {
      DBGMSGLN("[HTTP} Unable to connect");
    }
#else
    //
    // For ESP32
    //
    HTTPClient http;
    
    DBGMSGLN(strUrl);

    DBGMSG("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(strUrl); //HTTP

    DBGMSG("[HTTP] GET...\n");
    // start connection and send HTTP header
    httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        DBGMSGF("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            *payload = http.getString();
        }
    }
    else {
        DBGMSGF("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
#endif
  }
  else {
    DBGMSGLN("Unable to connect to WIFI AP.");
  }
  return httpCode;
}

int FwBoxSync::sendHttpsGet(const char* strUrl, String* payload) {
  int httpCode = 0x7fff;

  if ((WiFi.status() == WL_CONNECTED)) {
#if defined(ESP8266)

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  
    DBGMSGLN(strUrl);

    client->setInsecure();

    DBGMSGLN("Before https.begin");
    HTTPClient https;
    if (https.begin(*client, strUrl)) { // HTTPS
      DBGMSGLN("After https.begin");

      https.setTimeout(6 * 1000);
      DBGMSGLN("After https.setTimeout");
      httpCode = https.GET();
      DBGMSGLN("After https.GET");

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        DBGMSGF("[HTTPS] GET... code: %d\r\n", httpCode);
        // file found at server?
        if (httpCode == HTTP_CODE_OK) {
          (*payload) = https.getString();
          payload->trim();
          /*const char* cc = payload->c_str();
          if ((cc[0] == 0xef) && (cc[1] == 0xbb) && (cc[2] == 0xbf)) { // UTF-8 sign
            (*payload) = payload->substring(3); // Skip UTF-8 sign
          }*/
          //DBGMSGLN(*payload);
        }
      }
      else {
        DBGMSGF("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
    else {
      DBGMSGLN("[HTTPS] Unable to connect");
    }
#else
    //
    // For ESP32
    //

    //
    // For HTTPS, get root ca from NVS.
    //
    Preferences prefs;
    prefs.begin("FW_BOX"); // use "FW_BOX" namespace
    String curr_rootca = prefs.getString("RootCa", String());
    if (curr_rootca.length() <= 0) {
      DBGMSGLN("Root CA is empty.");
      return -1;
    }

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    DBGMSG("[HTTPS] begin...\n");
    DBGMSGLN(strUrl);
    if (https.begin(strUrl, curr_rootca.c_str())) {  // HTTPS
      DBGMSG("[HTTPS] GET...\n");
      // start connection and send HTTP header
      httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        DBGMSGF("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          *payload = https.getString();
          payload->trim();
          //DBGMSGLN(*payload);
        }
      }
      else {
        DBGMSGF2("[HTTPS] GET... failed, error: (%d) %s\n", httpCode, https.errorToString(httpCode).c_str());
      }

      https.end();
    }
    else {
      DBGMSG("[HTTPS] Unable to connect\n");
    }

#endif
  } // END OF "if((WiFi.status() == WL_CONNECTED))"

  return httpCode;
}

String FwBoxSync::encodeUrl(String* str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str->length(); i++){
      c=str->charAt(i);
      if (c == ' '){
        encodedString+= '+';
      }
      else if (isalnum(c)) {
        encodedString+=c;
      }
      else {
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

String FwBoxSync::encodeHash(const char* str)
{
    String re = "";
    int str_len = strlen(str);
    String str_mac = WiFi.macAddress(); // Example : F4:CF:A2:5B:B2:06
    str_mac.toUpperCase(); // Make sure all the letters are upper case.
    int mac_len = str_mac.length();
    //Serial.print("str_mac=");
    //Serial.println(str_mac);
    for (int si = 0; si < str_len; si++) {
      re += str[si];
      re += str_mac[si % mac_len];
    }
    //Serial.print("encodeHash:1=");
    //Serial.println(re);
    FwBoxUtil util;
    String re2 = util.toSha1(re.c_str());
    //Serial.print("encodeHash:2=");
    //Serial.println(re2);
    re2 = FwBoxSync::encodeUrl(&re2);
    return re2;
}
