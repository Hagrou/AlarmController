
#include "Vigil.h"


#define SWITCH_OFF_ALARM_URL DOMOTICZ_URL  "/json.htm?type=command&param=switchlight&idx=" ALARM_IDX "&switchcmd=Off"
#define GET_STATUS_URL DOMOTICZ_URL        "/json.htm?type=devices&rid=" SECURITY_LEVEL_IDX

#define SET_DISARMED_URL DOMOTICZ_URL      "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=0"
#define SET_ARMING_URL DOMOTICZ_URL        "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=10"
#define SET_ARMED_URL DOMOTICZ_URL         "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=20"
#define SET_WARNING_URL DOMOTICZ_URL       "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=30"
#define SET_ALARM_URL DOMOTICZ_URL         "/json.htm?type=command&param=switchlight&switchcmd=Set%20Level&idx=" SECURITY_LEVEL_IDX "&level=50"

//===========================================================  
Vigil::Vigil(Buzzer  *_pBuzzer, int pinData0, int pinData1) {
  rfidBus.begin(pinData0, pinData0, pinData1, pinData1);
  state=Disarmed;
  pBuzzer=_pBuzzer;
  timeUp=DOMOTICZ_LONG_DELAY;
  grantAccessTimeUp=DOMOTICZ_LONG_DELAY;
  poolFrequency=DOMOTICZ_LONG_DELAY;
  debugOn=true;
  SPIFFS.begin();
  FSInfo fsInfo;

  if (SPIFFS.exists("/config.dat")) {
    spiffsReadConfig();
  }
  else {
    spiffsSaveConfig();
  }
  spiffsStatus();
}

//===========================================================
bool Vigil::spiffsSaveConfig() {
  size_t szGrantDB=0;
  uint8_t buzzerFlag=0;

  // create a file, delete previous if it already exists, and open it for reading and writing
  Serial.printf("===== Writing /config.dat ====\n");
  File file = SPIFFS.open("/config.dat","w");
  if (!file) {
    Serial.printf("Error write SPIFFS file\n"); 
    return false;
  }
  buzzerFlag=pBuzzer->speaker()?1:0;
  file.write((uint8_t *) &buzzerFlag, sizeof(uint8_t));  // write buzzer status
  szGrantDB=grantDB.size();
  file.write((uint8_t *) &szGrantDB, sizeof(size_t));  // write grant size
  Serial.printf("write %d grants\n", szGrantDB);
  // write to it
  for (const auto &creds: grantDB) {
    file.write((uint8_t *) &creds.code, sizeof(uint32_t));  // write grant code
    file.write((uint8_t *) &creds.tagName, GRANT_TAG_SIZE); 
    Serial.printf("write [%s, %8x]\n", creds.tagName, creds.code);
  }

  // close it
  file.close();
  Serial.printf("================================\n");
  return true;
}

//===========================================================
bool Vigil::spiffsReadConfig() {
  uint8_t    buzzerFlag=0;
  size_t     szGrantDB=0;
  Credential creds;
  
  Serial.printf("===== Reading /config.dat ====\n");
  // TODO create a file, delete previous if it already exists, and open it for reading and writing
  File file = SPIFFS.open("/config.dat","r");
  if (!file) {
    Serial.printf("SPIFFS open file error\n");
    return false;
  }

  file.read((uint8_t *) &buzzerFlag, sizeof(uint8_t));  // write buzzer status
  if (buzzerFlag==0) { pBuzzer->shutUp(); }  // no sound
  else { pBuzzer->speakOn(); }
  
  file.read((uint8_t *) &szGrantDB, sizeof(size_t));  // read grant size  
  Serial.printf("found %d grants\n", szGrantDB);
  for (size_t i=0; i<szGrantDB; i++) {
    memset(&creds, 0, sizeof(Credential));
    file.read((uint8_t *) &creds.code, sizeof(uint32_t));  // write grant code
    file.read((uint8_t *) &creds.tagName, GRANT_TAG_SIZE); 
    grantDB.push_back(creds);                             // add grant in grantDB
    Serial.printf("%d: [%s, %8x]\n", i, creds.tagName, creds.code);
  }
  Serial.printf("================================\n");
  // close it
  file.close();
  return true;
}

//===========================================================
void Vigil::spiffsStatus() {
  FSInfo fsInfo;
 
  SPIFFS.info(fsInfo);  // info fs
  
  float total=(fsInfo.totalBytes/1024); //fsInfo.totalBytes
  Serial.println("info totalbytes : "+String(total));
  Serial.println("info usedbytes : "+String(fsInfo.usedBytes));
  Serial.println("info blocksize : "+String(fsInfo.blockSize));
  Serial.println("info pagesize: "+String(fsInfo.pageSize));
  Serial.println("info maxOpenFiles : "+String(fsInfo.maxOpenFiles));
  Serial.println("info maxPathLength : "+String(fsInfo.maxPathLength));  
}


//===========================================================
bool Vigil::grantAccess(uint32_t key, std::string &tagName, std::string &message) {
  if (Disarmed!=state) {
    message="Alarm Controller must be desarmed";
    return false;  
  }
  if (GRANT_MAX_SIZE==grantDB.size()) {
    message="GRANT MAX SIZE REACHED, grantAccess Aborted";
    return false;  
  }
  memset(&newCreds,0, sizeof(Credential));
  strncpy(newCreds.tagName, tagName.c_str(), GRANT_TAG_SIZE-1);
  
  state=GrantAccess;
  grantAccessTimeUp=0; // start timer for waiting rfid key
  pBuzzer->play('g', 100); pBuzzer->play('c', 100);
 
  return true;
}

//===========================================================
bool Vigil::grantRevoke(uint32_t tagId, std::string &message) {
  if (grantDB.size()<=tagId) {
    message="Invalid tagId, grantRevoke Aborted";
    return false;
  }
  grantDB.erase(grantDB.begin() + tagId);
  spiffsSaveConfig();
  return true;
}

//===========================================================
const Credential *Vigil::checkKey(uint32_t key) {
    for (const auto &validKey: grantDB) {
      
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

   
    if (GrantAccess==state) {
       newCreds.code=rfidBus.getCode();
       grantDB.push_back(newCreds); 
       spiffsSaveConfig();
       state=Disarmed;
       pBuzzer->play('c', 100); pBuzzer->play('g', 100);
       return;
    }
    const Credential *cred=checkKey(rfidBus.getCode());
    if (nullptr!=cred) {
      Serial.printf("\nACCESS GRANTED FOR %s\n", cred->tagName);
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
    case GrantAccess:
      pBuzzer->ledGreen();
      pBuzzer->play('c', 20);
      pBuzzer->ledBlue();   
      break;
    default:
      break;
   }
}

//===========================================================
void Vigil::loop() {
  checkAccess();
  
  if (GrantAccess==state) { showAlive(); }
  else {
    if (timeUp >= poolFrequency) {
      getSecurityDeviceStatus();
      timeUp = 0;  // reset the counter to 0 so the counting starts over...
    }
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
    case GrantAccess:
      if (grantAccessTimeUp>= poolFrequency) {
        state=Disarmed;
        Serial.printf("Too late GrantAccess -> Disarmed\n"); 
      }
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

//===========================================================
std::string Vigil::grantToHtml() {
  char        buffer[128];
  uint32_t    id=0;
  std::string page="<h2>Grand List</h2>";
 
  memset(buffer, 0, 128);
  for (const auto &creds: grantDB) {

    // uint32_t     code;
    // char         tagName[GRANT_TAG_SIZE]; 

    page+="  <form action='/grantRevoke'>\n";

    snprintf(buffer,127, "    <input type='hidden' name='id' value='%d'>\n", id);  page+=buffer;
    snprintf(buffer,127, "    Name: <input type='text' name='fname' value='%s' disabled/>\n", creds.tagName); page+=buffer;
    snprintf(buffer,127, "    Code: <input type='text' name='lname' value='%x' disabled/>\n", creds.code); page+=buffer;
    page+=
      "    <input type='submit' value='del'/><br/>\n"
      "  </form>\n";
      
    id++;
  }
  page+=
      "  <h3>Register rfid Key</h3>\n"
      "  <form action='/grantAccess'>\n";

  snprintf(buffer,127, "     Name: <input type='text' name='tagName' pattern='[a-zA-Z]{,%d}' required/>\n", GRANT_TAG_SIZE);  page+=buffer;
  page+=   
      "     <input type='submit' value='add'/>\n"
      "  </form>\n" ;
  return page;
}