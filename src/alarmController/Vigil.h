#pragma once

#include <elapsedMillis.h>

#include <ESP8266HTTPClient.h>

#include "Wiegand.h"
#include "config.h"
#include "Buzzer.h"
 

typedef struct {
  uint32_t     code;
  std::string name;
} Credential;

class Vigil {
  typedef enum { Disarmed=0, Arming=10, Armed=20, Warn1=30, Warn2=40, Alarm=50 } State;
  
public:  Vigil(Buzzer *pBuzzer, int pinData0, int pinData1);  
  
  void checkAccess();
  bool getSecurityDeviceStatus();             // get from domoticz security state
  bool setSecurityDeviceStatus(State newState);
  bool switchOffAlarm();
  State getState();
  void loop();
private:
  WIEGAND           rfidBus;
  State             state;                   
  Buzzer            *pBuzzer;
  elapsedMillis      timeUp;
  unsigned int       poolFrequency;
  bool               debugOn;
  void               showAlive();
  const Credential  *checkKey(uint32_t key);   // search if key registered in grandList
};
