#pragma once
                                    
class Buzzer {
  public:
    Buzzer(int _pinBuzzer, int _pinLed);

    void playDisarmed();                // play and show disarmed state
    void playArming();                  // play and show arming state
    void playArmed();                   // play and show armed state
    void playWarning();                 // play and show warning state
    void playAlarm();                   // play and show alarm state
    void play(char note, int duration); // play a note
    bool speaker();                     // return speaker status
    void shutUp();                      // no sound
    void speakOn();                     // active sound
    void ledBlue();                     // switch led to blue
    void ledGreen();                    // switch led to green
  protected:
    void tone(int freq, int duration);
    bool isOn;
    int  pinBuzzer;
    int  pinLed;
};
