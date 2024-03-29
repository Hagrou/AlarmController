#pragma once

#define PIN_DATA0  D2                       // GREEN : DATA0 weigand 
#define PIN_DATA1  D3                       // WHITE : DATA1 Weigand 
#define PIN_BUZZER D6 
#define PIN_LED    D5                       // BLUE: LED 
#define COM_SPEED  115200                   // Serial Com Speed for Debugging
#define GRANT_TAG_SIZE       16             // max key name size
#define GRANT_MAX_SIZE       32             // max registered keys
#define WWW_USERNAME         "#adminName#"  // admin authentication
#define WWW_PASSWORD         "#adminPwd#"   

#define DOMOTICZ_LONG_DELAY  10*1000        // get state each 10 sec
#define DOMOTICZ_SHORT_DELAY 1*1000     

#define WIFI_SSID            "#wifiSSID#"          // wifi SSID
#define WIFI_PASSWORD        "#wifiPassword#"      // wifi password

#define OTA_HOST             "#otaHost#"           // wifi upgrde configuraiton
#define OTA_PASSWORD         "#otaPassword#"
#define OTA_PORT             #otaPort#

#define JEEDOM_URL           "#jeedomURL#"          // jeedom  url
#define JEEDOM_API_KEY       "#jeedomApiKey#"       // jeedom api key
#define JEEDOM_CMD_ID        "#jeedomCmdId#"        // jeedom cmd id

#define SECURITY_LEVEL_IDX   "#securityLevelIDX#"   // domoticz security level device idx

#define HTTP_SERVER_PORT     #adminPort#

