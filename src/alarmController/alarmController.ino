
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "Wiegand.h"
#include "config.h"
#include "Buzzer.h"
#include "Vigil.h"

Buzzer            *pBuzzer=nullptr;
Vigil             *pVigil=nullptr;
ESP8266WebServer   server(HTTP_SERVER_PORT); 


//===========================================================
void handleStatus() {
  char     buffer[256];
  uint32_t free=0;
  uint16_t max=0;
  uint8_t  frag=0;
  
  memset(buffer,0,sizeof(buffer));
  ESP.getHeapStats(&free, &max, &frag);
  snprintf(buffer,sizeof(buffer)-1, "Heap: %5d free: %5d - max: %5d - frag: %3d%%\nSpeaker:%s State=%d", 
           ESP.getFreeContStack(), free, max, frag,pBuzzer->speaker()?"On":"Off", (int) pVigil->getState());
  server.send(200, "text/plain", buffer);
}

//===========================================================
void handleSpeakerOn() {
  pBuzzer->speakOn();
  server.send(200, "text/plain", "OK");
}

//===========================================================
void handleSpeakerOff() {
  pBuzzer->shutUp();
  server.send(200, "text/plain", "OK");
}
  
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

//==========================================================
void setup() {
  Serial.begin(COM_SPEED);
  Serial.print("AlarmController Starting..\n");
  
  pBuzzer=new Buzzer(PIN_BUZZER, PIN_LED);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("\tWaiting for WIFI connection...");
  // Wait for connection

  pBuzzer->ledBlue();
  while (WiFi.status() != WL_CONNECTED) {
    pBuzzer->ledBlue();
    delay(50);
    Serial.print(".");
    pBuzzer->ledGreen();
    delay(50);
  }
  pBuzzer->ledGreen();
  Serial.printf("\n\tConnected to '%s' with @ip ", WIFI_SSID);
  Serial.println(WiFi.localIP());
  

  pVigil=new Vigil(pBuzzer, PIN_DATA0,PIN_DATA1);

  if (!pVigil->getSecurityDeviceStatus()) {
    Serial.printf("\n\tgetSecurityStatus Error"); 
  }

  server.on("/status",      handleStatus);
  server.on("/speakerOn",   handleSpeakerOn);
  server.on("/speakerOff",  handleSpeakerOff);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("\n\tHTTP server started");
  Serial.print("Init Done\n");
  pBuzzer->speakOn();
  // pBuzzer->shutUp();
}

void loop() {
  server.handleClient();
  pVigil->loop();

  delay(500);
}
