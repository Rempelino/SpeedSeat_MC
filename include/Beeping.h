#ifndef BEEP_H
#define BEEP_H
#include "Arduino.h"

class Beeping
{
private:
    const int Pin;
    bool beeperOn;
    const unsigned long beepingIntervall;
    const unsigned long pauseBetweenBeeping;
    unsigned long timeStampLastBeepFinished;
    bool pauseActive;
    int beepsLeft;
    unsigned long timeStamp;

public:
    Beeping(int Pin, int Intervall, int Pause);
    void beep(int amountOfBeeps);
    void doubleBeep();
    void kill();
    bool beepingActive;
};

Beeping::Beeping(int Pin, int Intervall, int Pause)
    : Pin(Pin),
      beepingIntervall(Intervall),
      pauseBetweenBeeping(Pause)

{
    pinMode(Pin, OUTPUT);
    digitalWrite(Pin, LOW);
}

void Beeping::beep(int amountOfBeeps)
{
    if (pauseActive)
    {
        if (millis() - timeStampLastBeepFinished > pauseBetweenBeeping)
        {
            pauseActive = false;
        }
        else
        {
            return;
        }
    }

    if (!beepingActive)
    {
        if (amountOfBeeps == 0)
        {
            return;
        }
        beepingActive = true;
        beeperOn = true;
        beepsLeft = amountOfBeeps;
        timeStamp = millis();
        digitalWrite(Pin, HIGH);
        return;
    }
    if (millis() - timeStamp > (beepingIntervall / 2))
    {   
        timeStamp = millis();
        if (beeperOn)
        {
            digitalWrite(Pin, LOW);
            beeperOn = false;
            beepsLeft--;
            if (beepsLeft == 0)
            {
                beepingActive = false;
                pauseActive = true;
                timeStampLastBeepFinished = millis();
            }
        }
        else
        {
            digitalWrite(Pin, HIGH);
            beeperOn = true;
        }
    }
}

void Beeping::kill()
{
    digitalWrite(Pin, LOW);
    beeperOn = false;
    beepsLeft = 0;
    beepingActive = false;
    pauseActive = false;
}

void Beeping::doubleBeep()
{
    digitalWrite(PIN_BEEPER, HIGH);
    delay(100);
    digitalWrite(PIN_BEEPER, LOW);
    delay(100);
    digitalWrite(PIN_BEEPER, HIGH);
    delay(100);
    digitalWrite(PIN_BEEPER, LOW);
    delay(1000);
}

#endif