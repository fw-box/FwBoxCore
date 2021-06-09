//
// Copyright (c) 2021 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//

//
// Default IP address of AP is "192.168.4.1"
//

#ifndef __FWBOXCFGSERVER__
#define __FWBOXCFGSERVER__

#include <Arduino.h>
#include "BuildSettings.h"
#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  ESP8266WebServer CfgServer(80);
#elif defined(ESP32)
  //
  // ESP32
  //
  //#include <Preferences.h>
  #include <WebServer.h>
  WebServer CfgServer(80);
#elif defined(CONFIG_PLATFORM_8195A)
  #include <WiFi.h>
  WiFiServer CfgServer(80);
#endif


extern FwBoxCore FwBoxIns; // FwBoxCore instance

const char HTML_CFG_SERVER[] PROGMEM = "<!DOCTYPE html><html><head><title>fw-box</title></head><body><center><h1>fw-box.com</h1><form action='/' method='post'><table><tr><td>WIFI SSID</td><td><input type='text' name='ssid'></td></tr><tr><td>WIFI PASSWORD</td><td><input type='password' name='pw'></td></tr></tr><tr><td>Email</td><td><input type='text' name='email'></td></tr><tr><td colspan='2'><input type='submit'></td></tr></table></form></center></body></html>";
const char HTML_CFG_SERVER_CONNECTING[] PROGMEM = "<!DOCTYPE html><html><head><title>fw-box</title><meta http-equiv='refresh' content='5; URL=/?state=6'></head><body><center><h1 style='color:blue;'>Connecting...</h1></center></body></html>";

void handleRoot() {
  String wifi_ssid = "";
  String wifi_pw = "";
  String state = "";
  for (uint8_t i = 0; i < CfgServer.args(); i++) {
    if(CfgServer.argName(i).equals("ssid") == true) {
      wifi_ssid = CfgServer.arg(i);
    }
    else if(CfgServer.argName(i).equals("pw") == true) {
      wifi_pw = CfgServer.arg(i);
    }
    else if(CfgServer.argName(i).equals("email") == true) {
      FwBoxIns.setEmail(CfgServer.arg(i).c_str());
    }
    else if(CfgServer.argName(i).equals("state") == true) {
      state = CfgServer.arg(i);
    }
  }
  
  if(wifi_ssid.length() > 0) {

    DBGMSG("wifi_ssid=");
    DBGMSGLN(wifi_ssid);
    DBGMSG("wifi_pw=");
    DBGMSGLN(wifi_pw);

#if defined(ESP32)
    //
    // Save the SSID and password to NVS.
    //
  	/*Preferences prefs;
    prefs.begin("FW_BOX_WIFI"); // use "WIFI" namespace
    prefs.putString("SSID", wifi_ssid.c_str());
    prefs.putString("PASS", wifi_pw.c_str());*/
#endif
    
    WiFi.begin(wifi_ssid.c_str(), wifi_pw.c_str());

    CfgServer.send(200, "text/html", HTML_CFG_SERVER_CONNECTING);

    return;
  }
  else if(state.equals("6") == true) {
    if(WiFi.status() != WL_CONNECTED) {
      CfgServer.send(200, "text/html", HTML_CFG_SERVER_CONNECTING);
      return;
    }
  }

  if(WiFi.status() == WL_CONNECTED) {
    String str_to = "<center><h1 style='color:blue;'>WiFi connected</h1>";
    if((FwBoxIns.getDevName().length() > 0) || (FwBoxIns.getValCount() > 0)) {
      if(FwBoxIns.getEmail().length() > 0) {
        str_to += "<h1 style='color:blue;'>Device joined - ";
        str_to += FwBoxIns.getEmail();
        str_to += "</h1>";
      }
    }
    str_to += "</center>";
    CfgServer.send(200, "text/html", str_to);
  }
  else
    CfgServer.send(200, "text/html", HTML_CFG_SERVER);

}

void cfgServerBegin()
{
  //DBGMSGLN("cfgServerBegin:03");

  CfgServer.on("/", handleRoot);
  CfgServer.begin();
  
  //DBGMSGLN("cfgServerBegin:04");
}

void cfgServerHandle()
{
  CfgServer.handleClient();
}

#endif // __FWBOXCFGSERVER__
