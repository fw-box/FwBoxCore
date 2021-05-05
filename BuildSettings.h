//
// Copyright (c) 2021 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//

#ifndef __BUILDSETTINGS__
#define __BUILDSETTINGS__

#define ARDUINOJSON_DECODE_UNICODE 1
#define MAX_DEV_NAME_LENGTH 100


#define COMPANY "FW-BOX"

#define DEFAULT_SYSCFG_NTPSERVER "pool.ntp.org"
#define DEFAULT_SYSCFG_TIMEZONE 8

#define DEFAULT_SYSCFG_SYNC_INTERVAL 180*1000 // MilliSeconds
#define DEFAULT_SYSCFG_SYNC_INTERVAL_SHORT 20*1000 // MilliSeconds

#define DEFAULT_SYSCFG_FASTCMD_INTERVAL 30*1000 // MilliSeconds
#define DEFAULT_SYSCFG_FASTCMD_INTERVAL_SHORT 3*1000 // MilliSeconds

#define DEFAULT_MQTT_PUBLISH_INTERVAL 20 * 1000 // MilliSeconds
#define DEFAULT_CALL_A_URL_INTERVAL 20 * 1000 // MilliSeconds


//
// After timeout, restore the values - "SyncInterval"="DEFAULT_SYSCFG_SYNC_INTERVAL" and "FastCmdInterval"="DEFAULT_SYSCFG_FASTCMD_INTERVAL".
//
#define TIMEOUT_USER_IS_ACTIVE 60 * 1000 // MilliSeconds

/*
    define('CMD_DEV_REQUEST_FIRMWARE_INFO', 20);
    define('CMD_DEV_FW_UD_STAGE', 25);
*/
#define CMD_DEV_REQUEST_FIRMWARE_INFO 20
#define CMD_DEV_FW_UD_STAGE 25


//
// define('FIRMWARE_UPDATE_FORCE', 1);
// define('FIRMWARE_UPDATE_OFFICIAL_VERSION', 2);
//
#define FIRMWARE_UPDATE_FORCE 1
#define FIRMWARE_UPDATE_OFFICIAL_VERSION 2

//
// define('FW_UD_STAGE_USER_SENT_REQUEST', 1);
// define('FW_UD_STAGE_DEVICE_STARTED_UPDATING', 2);
// define('FW_UD_STAGE_ERROR', 3);
// define('FW_UD_STAGE_DONE', 127);
//
#define FW_UD_STAGE_USER_SENT_REQUEST 1
#define FW_UD_STAGE_DEVICE_STARTED_UPDATING 2
#define FW_UD_STAGE_ERROR 3
#define FW_UD_STAGE_DONE 127

//
// Function support
//
#define ENABLE_HTTPS_UPDATE 1
#define ENABLE_IIC 0 // Default disabled, it would be enabled by sensor's define.
#define ENABLE_ONE_WIRE 0 // Default disabled, it would be enabled by sensor's define.
#define ENABLE_CONFIG_SERVER 0
#define ENABLE_MQTT 1


#if ENABLE_MQTT == 1
  //
  // Please refer to database table - 'devices'
  //
  //define('MQTT_TOPIC_TYPE_HOME_ASSISTANT', 1);
  //define('MQTT_TOPIC_TYPE_THING_SPEAK', 2);
  //define('MQTT_TOPIC_TYPE_IDEAS_CHAIN', 3);
  //define('MQTT_TOPIC_TYPE_CUSTOM', 4);
  #define MQTT_TOPIC_TYPE_HOME_ASSISTANT 1
  #define MQTT_TOPIC_TYPE_THING_SPEAK 2
  #define MQTT_TOPIC_TYPE_IDEAS_CHAIN 3
  #define MQTT_TOPIC_TYPE_CUSTOM 4
#endif // #if ENABLE_MQTT == 1


#define DEBUG 0


#define URL_PATH_CONN "/conn.php"
#define URL_PATH_SYNC_DATA "/sync.php"
#define URL_PATH_UPLOAD "/upload"
#define URL_PATH_FIRMWARE "/fw"
#define URL_PATH_FAST_CMD "/fast_cmd"
#define URL_PATH_DEV_CMD "/cmd_dev_from_dev.php"
#define URL_PATH_ROOT_CA "/az/cmd__.php"

#define MAX_VALUE_COUNT 8

#if DEBUG == 1
  #define DBGMSG(VAL) Serial.print(VAL)
  #define DBGMSGLN(VAL) Serial.println(VAL)
  #define DBGMSGF(FORMAT, ARG) Serial.printf(FORMAT, ARG)
  #define DBGMSGF2(FORMAT, ARG1, ARG2) Serial.printf(FORMAT, ARG1, ARG2)
  #define PRINT_VAR(X) Serial.print(#X);Serial.print(" = ");Serial.println(X);
#else
  #define DBGMSG(VAL)
  #define DBGMSGLN(VAL)
  #define DBGMSGF(FORMAT, ARG)
  #define DBGMSGF2(FORMAT, ARG1, ARG2)
  #define PRINT_VAR(X)
#endif

#endif // __BUILDSETTINGS__
