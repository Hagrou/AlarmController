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

#define DOMOTICZ_URL         "#domoticzURL#"       // domoticz url
#define SECURITY_LEVEL_IDX   "#securityLevelIDX#"  // domoticz security level device idx
#define ALARM_IDX            "#alarmIDX#"          // Alarm idx
#define HTTP_SERVER_PORT     #controllerServerPort#

