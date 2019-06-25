
#include "Vigil.h"


#define SWITCH_OFF_ALARM_URL DOMOTICZ_URL  "/json.htm?type=command&param=switchlight&idx=" ALARM_IDX "&switchcmd=Off"
#define GET_STATUS_URL DOMOTICZ_URL        "/json.htm?type=devices&rid=" SECURITY_LEVEL_IDX

#define SET_DISARMED_URL DOMOTICZ_URL      "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=0"
#define SET_ARMING_URL DOMOTICZ_URL        "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=10"
#define SET_ARMED_URL DOMOTICZ_URL         "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=20"
#define SET_WARNING_URL DOMOTICZ_URL       "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=30"
#define SET_ALARM_URL DOMOTICZ_URL         "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=50"

Credential grantList[]={ ACCESS_LIST };

//===========================================================
Vigil::Vigil(Buzzer  *_pBuzzer,int pinData0, int pinData1) {
  rfidBus.begin(pinData0, pinData0, pinData1, pinData1);
  state=Disarmed;
  pBuzzer=_pBuzzer;
  timeUp=DOMOTICZ_LONG_DELAY;
  poolFrequency=DOMOTICZ_LONG_DELAY;
  debugOn=true;
}

//===========================================================
const Credential *Vigil::checkKey(uint32_t key) {
    for (const auto &validKey: grantList) {
      if (key==validKey.code) return &validKey;
    }
    return nullptr;
}

//===========================================================
void Vigil::checkAccess() {
  if(rfidBus.available()) {
    Serial.print("Wiegand HEX = ");
    Serial.print(rfidBus.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(rfidBus.getCode());
    Serial.print(", Type W");
    Serial.print(rfidBus.getWiegandType());   

    const Credential *cred=checkKey(rfidBus.getCode());
    if (nullptr!=cred) {
      Serial.printf("\nACCESS GRANTED FOR %s\n", cred->name.data());
      pBuzzer->play('c', 100);pBuzzer->play('d', 100);pBuzzer->play('e', 100);
      
      State newState=Disarmed;
      switch(state) {
      case Disarmed:
        newState=Arming;
        pBuzzer->play('c', 100);pBuzzer->play('d', 100);pBuzzer->play('e', 100);
        switchOffAlarm();
        break;
      case Arming: 
      case Warn1:
      case Warn2:
      case Armed:    
      case Alarm:    
        newState=Disarmed;
        pBuzzer->play('c', 100);pBuzzer->play('f', 100);pBuzzer->play('g', 100);
        break;
      }
      setSecurityDeviceStatus(newState);
    }
    else {
         Serial.println("ACCESS DENIED");
         pBuzzer->play('a', 50);  delay(20); pBuzzer->play('a', 50);
    }
  }
}

//===========================================================


bool Vigil::getSecurityDeviceStatus() {
  HTTPClient  httpClient;
  bool        isValid=true;
      
  Serial.printf("getCmd %s\n", (char *) GET_STATUS_URL);
  httpClient.begin(GET_STATUS_URL); // set http
  int httpCode = httpClient.GET();  // start connection and send HTTP header
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("get %s: %d\n", GET_STATUS_URL,httpCode);
    if (HTTP_CODE_OK==httpCode) {
      String data=httpClient.getString();
      std::size_t pos = data.indexOf("\"Level\" :");
      String sState= data.substring(pos+10,pos+12);     // get from "live" to the end
    
      Serial.printf("Level=[%s] (%d)\n", sState.c_str(), sState.toInt()); // TODO: del
      state=(State) sState.toInt();
      
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
      isValid=false;
    }
  }
  httpClient.end(); // close request
  showAlive(); 
  return isValid;
}
//===========================================================
Vigil::State Vigil::getState() {
  return state;
}

//===========================================================
void Vigil::showAlive() {
   switch (state) {
    case Disarmed:
      pBuzzer->ledBlue();
      delay(50);
      pBuzzer->ledGreen();
      break;
    case Armed:
      pBuzzer->ledGreen();
      delay(50);
      pBuzzer->ledBlue();
      break;
    default:
      break;
   }
}

//===========================================================
void Vigil::loop() {
  checkAccess();
  if (timeUp >= poolFrequency) {
    getSecurityDeviceStatus();
    timeUp = 0;  // reset the counter to 0 so the counting starts over...
  }
  switch (state) {
    case Disarmed:
      pBuzzer->playDisarmed();
      poolFrequency=DOMOTICZ_LONG_DELAY;
      break;
    case Arming:
      pBuzzer->playArming();
      poolFrequency=DOMOTICZ_SHORT_DELAY;
      break;
    case Armed:
      pBuzzer->playArmed();
      poolFrequency=DOMOTICZ_LONG_DELAY;
      break;
    case Warn1:
    case Warn2:
      pBuzzer->playWarning();
      poolFrequency=DOMOTICZ_SHORT_DELAY;
      break;
    case Alarm:
      pBuzzer->playAlarm();
      poolFrequency=DOMOTICZ_SHORT_DELAY;
      break;
  }
}

//===========================================================
bool Vigil::setSecurityDeviceStatus(State newState) {
  bool        isValid=true;
  HTTPClient  httpClient;
  
  switch(newState) {
  case Disarmed: httpClient.begin(SET_DISARMED_URL); break;
  case Arming:   httpClient.begin(SET_ARMING_URL); break;
  case Armed:    httpClient.begin(SET_ARMED_URL); break;
  case Warn1:    httpClient.begin(SET_WARNING_URL); break;
  case Warn2:    break; // nothing to do
  case Alarm:    httpClient.begin(SET_ALARM_URL); break;
  }
  int httpCode = httpClient.GET();  // start connection and send HTTP header
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    if (HTTP_CODE_OK==httpCode) {
      Serial.print(httpClient.getString());
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
      isValid=false;
    }
  }
  httpClient.end(); // close request

  return isValid;
}

//===========================================================
bool Vigil::switchOffAlarm() {
  bool        isValid=true;
  HTTPClient  httpClient;
  
  httpClient.begin(SWITCH_OFF_ALARM_URL);
  int httpCode = httpClient.GET();  // start connection and send HTTP header
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    if (HTTP_CODE_OK==httpCode) {
      Serial.print(httpClient.getString());
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
      isValid=false;
    }
  }
  httpClient.end(); // close request

  return isValid;
}
