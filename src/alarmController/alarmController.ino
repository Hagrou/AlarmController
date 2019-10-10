
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
void handleBuzzer() {
  if (!server.authenticate(WWW_USERNAME, WWW_PASSWORD)) {
      return server.requestAuthentication();
  }
  
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i)=="set") {
      if (server.arg(i)=="on") {
        Serial.printf("Buzzer speakOn\n");
        pBuzzer->speakOn();
      }
      else {
        Serial.printf("Buzzer shutup\n");
        pBuzzer->shutUp(); 
      }
    }
  }
  server.send(200, "text/html",mainHtmlPage().c_str());
  pVigil->spiffsSaveConfig();
}

//===========================================================
void handleGrantRevoke() {
  uint32_t     tagId=UINT_MAX;
  std::string  message;
  
  if (!server.authenticate(WWW_USERNAME, WWW_PASSWORD)) {
      return server.requestAuthentication();
  }

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i)=="id") {
      tagId=atoi(server.arg(i).c_str()); 
    }
  }

  if (tagId==-1) {
    server.send(500, "text/plain", "id required");
    return;
  }
  
  if (pVigil->grantRevoke(tagId,message)) {
    server.send(200, "text/html", mainHtmlPage().c_str());
  }
  else {
    server.send(500, "text/plain", message.c_str());
  }
}

//===========================================================
void handleGrantAccess() {
  std::string  tagName;
  std::string  message;
  bool         isValidPassword=false;
  
  if (!server.authenticate(WWW_USERNAME, WWW_PASSWORD)) {
      return server.requestAuthentication();
  }
  
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i)=="tagName") {
      tagName=std::string(server.arg(i).c_str()); // !!!
    }
  }

  if (tagName.size()==0) {
    server.send(500, "text/plain", "tagName required");
    return;
  }
  
  if (pVigil->grantAccess(1, tagName,message)) {
    server.send(200, "text/html","<html><p>Waiting for RFID Key...</p></body></html>");
  }
  else {
    server.send(500, "text/plain", message.c_str());
  }
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

 //===========================================================
std::string mainHtmlPage() {
  char        buffer[256];
  uint32_t    free=0;
  uint16_t    max=0;
  uint8_t     frag=0;
  
  std::string main=
  "<html>\n"
  "  <meta http-equiv='refresh' content='10'/>"
  "  <body>\n"
  "    <h1>Alarm Controller</h1>\n"
  "    <h3>Status</h3>\n";

  memset(buffer,0,sizeof(buffer));
  ESP.getHeapStats(&free, &max, &frag);
  snprintf(buffer,sizeof(buffer)-1, "<p>Heap: %5d free: %5d - max: %5d - frag: %3d%%\n State=%d</p>\n", 
           ESP.getFreeContStack(), free, max, frag, (int) pVigil->getState());
  
  main+=buffer;
  main+=pBuzzer->buzzerToHtml();
  main+=pVigil->grantToHtml();
  main+=
  "  </body>"
  "</html>";
   return main;
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
  // http basic authentication
  server.on("/", []() {
    if (!server.authenticate(WWW_USERNAME, WWW_PASSWORD)) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", mainHtmlPage().c_str());
  });
  server.on("/buzzer",      handleBuzzer);
  server.on("/grantRevoke", handleGrantRevoke);
  server.on("/grantAccess", handleGrantAccess);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("\n\tHTTP server started");
  Serial.print("Init Done\n");
}

void loop() {
  server.handleClient();
  pVigil->loop();

  delay(500);
}
