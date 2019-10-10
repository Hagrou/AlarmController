#pragma once
#include <ESP8266HTTPClient.h>
#include <elapsedMillis.h>
#include <vector>
#include <FS.h>   //Include SPIFFS
#include "Wiegand.h"

#include "config.h"
#include "Buzzer.h"

typedef struct {
  uint32_t     code;
  char         tagName[GRANT_TAG_SIZE]; 
} Credential;

class Vigil {
  typedef enum { Disarmed=0, Arming=10, Armed=20, Warn1=30, Warn2=40, Alarm=50, GrantAccess=60 } State;
  
public:  
  Vigil(Buzzer *pBuzzer, int pinData0, int pinData1);  

  bool spiffsSaveConfig();  // save configuration into spiffs memory
  void spiffsStatus();      // print spffis memory status
  bool spiffsReadConfig();

  bool grantAccess(uint32_t key, std::string &tagName, std::string &msg); // register a new rfi tag 
  bool grantList(std::string &msg);                                      // show all creds
  bool grantRevoke(uint32_t tagId, std::string &message);    // revoke a rfi tag

  
  void checkAccess();
  bool getSecurityDeviceStatus();             // get from domoticz security state
  bool setSecurityDeviceStatus(State newState);
  bool switchOffAlarm();
  State getState();
  std::string grantToHtml();
  void loop();
 

private:
  WIEGAND                  rfidBus;
  State                    state;                   
  Buzzer                  *pBuzzer;
  elapsedMillis            timeUp;
  elapsedMillis            grantAccessTimeUp;
  unsigned int             poolFrequency;
  bool                     debugOn;
  std::vector<Credential>  grantDB;
  Credential               newCreds;   // use for credential management
  void                     showAlive();
  const Credential        *checkKey(uint32_t key);   // search if key registered in grandList
};
