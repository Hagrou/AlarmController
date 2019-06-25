#include <arduino.h>
#include "Buzzer.h"

//===========================================================
Buzzer::Buzzer(int _pinBuzzer, int _pinLed) {
  pinBuzzer=_pinBuzzer;
  pinLed=_pinLed;
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinLed, OUTPUT);
  shutUp();
}

//===========================================================
void Buzzer::tone(int frequency, int duration) {
  if (!isOn) return;
  for (long i = 0; i < duration * 1000L; i += frequency * 2) {
    digitalWrite(pinBuzzer, HIGH);
    delayMicroseconds(frequency);
    digitalWrite(pinBuzzer, LOW);
    delayMicroseconds(frequency);
  }
  digitalWrite(pinBuzzer, HIGH);
}

//===========================================================
void Buzzer::shutUp() {
  isOn=false;
  digitalWrite(pinBuzzer, HIGH);
}

//===========================================================
void Buzzer::speakOn() {
  isOn=true;
}

//===========================================================
bool Buzzer::speaker() {
  return isOn;
}

//===========================================================
void Buzzer::ledBlue() {
  digitalWrite(pinLed, HIGH);
}

//===========================================================
void Buzzer::ledGreen() {
  digitalWrite(pinLed, LOW);
}

//===========================================================
void Buzzer::playDisarmed() {
   digitalWrite(pinLed, LOW);
}

//===========================================================
void Buzzer::playArmed(){
   digitalWrite(pinLed, HIGH);
}
//===========================================================
void Buzzer::playArming() {
  digitalWrite(pinLed, HIGH);
  delay(100); 
  digitalWrite(pinLed, LOW);
  play('d', 100);
}

//===========================================================
void Buzzer::playWarning() {
  digitalWrite(pinLed, LOW);
  delay(100); 
  digitalWrite(pinLed, HIGH);
  play('a', 100);
}

//===========================================================
void Buzzer::playAlarm() {
   digitalWrite(pinLed, LOW);
   delay(100); 
   digitalWrite(pinLed, HIGH);
   delay(100); 
   play('a', 100);play('b', 100);play('c', 100);
}

//===========================================================
void Buzzer::play(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  char buffer[256];
    
  if (!isOn) return;

  sprintf(buffer, "Play %c\n", note);
  Serial.print(buffer);

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      tone(tones[i], duration);
    }
  }
}
