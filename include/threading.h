
#ifndef THREADING_H
#define THREADING_H
#include "Arduino.h"

#define MAXIMUM_AMOUNT_OF_THREADS 100

struct threadData
{
    void (*functionPointerVoid)();
    bool (*functionPointerBool)();
    bool threadAlive;
    bool threadIsVoid;
    bool threadIsBool;
    unsigned long executeIntervall;
    unsigned long timeTillKill;
    unsigned long timeTillNextExecute;
    bool timeOut;
    bool selfDestroy;
    unsigned long ID;
};

class threading
{
private:
    static threadData data[MAXIMUM_AMOUNT_OF_THREADS];
    static bool hasBeenInitialized;
    static bool processing;
    static unsigned long IDcounter;
    static unsigned long timeStampLastExecute;

    unsigned long ID;
    void initialize();
    int getFirstFreeThread();
    unsigned long getNewID();
    unsigned long getThreadByID();
    unsigned long getTimeTillNextStep();

public:
    threading(void (*functionPointer)(), unsigned long executeIntervall, unsigned long executeDuration, bool timeOut);
    threading(bool (*functionPointer)(), unsigned long executeIntervall, bool selfDestroy);
    threading();
    void execute();
    void kill();
    bool IsAlive();
    static unsigned long amountOfExecuteJumps;
};
threading threadInitializer;
ISR(TIMER1_COMPA_vect)
{
  threadInitializer.execute();
}

threadData threading::data[MAXIMUM_AMOUNT_OF_THREADS] = {0};
bool threading::hasBeenInitialized = false;
bool threading::processing = false;
unsigned long threading::IDcounter = 0;
unsigned long threading::timeStampLastExecute = 0;
unsigned long threading::amountOfExecuteJumps = 0;

threading::threading(){
    processing = true;
    initialize();
    processing = false;
}

threading::threading(void (*functionPointer)(), unsigned long executeIntervall = 1, unsigned long executeDuration = 10000, bool timeOut = false)
{
    processing = true;
    initialize();
    int x = getFirstFreeThread();
    this->ID = getNewID();
    this->data[x].executeIntervall = executeIntervall;
    this->data[x].timeTillKill = executeDuration;
    this->data[x].timeTillNextExecute = 0;
    this->data[x].timeOut = timeOut;
    this->data[x].functionPointerVoid = functionPointer;
    this->data[x].threadIsVoid = true;
    this->data[x].threadAlive = true;
    this->data[x].ID = ID;
    processing = false;
}

threading::threading(bool (*functionPointer)(), unsigned long executeIntervall = 1, bool selfDestroy = false)
{
    processing = true;
    initialize();
    int x = getFirstFreeThread();
    this->ID = getNewID();
    this->data[x].functionPointerBool = functionPointer;
    this->data[x].executeIntervall = executeIntervall;
    this->data[x].timeOut = false;
    this->data[x].threadIsBool = true;
    this->data[x].threadAlive = true;
    this->data[x].selfDestroy = selfDestroy;
    this->data[x].ID = ID;
    processing = false;
}

void threading::execute()
{
    if (amountOfExecuteJumps > 0)
    {
        amountOfExecuteJumps--;
        return;
    }
    unsigned long timeStampThisExecute = millis();
    unsigned long elapsedTime = timeStampThisExecute - timeStampLastExecute;
    int x = 0;
    while (data[x].threadAlive)
    {
        bool killThisThread = false;
        if (elapsedTime >= data[x].timeTillNextExecute)
        {
            while (elapsedTime >= data[x].timeTillNextExecute)
            {
                data[x].timeTillNextExecute = data[x].timeTillNextExecute + data[x].executeIntervall;
                if (data[x].executeIntervall == 0)
                {
                    break;
                }
            }
            if (data[x].threadIsVoid)
            {
                (*data[x].functionPointerVoid)();
            }
            else if (data[x].threadIsBool)
            {
                if ((*data[x].functionPointerBool)() && data->selfDestroy)
                {
                    killThisThread = true;
                }
            }
        }
        if (elapsedTime >= data[x].timeTillKill && data[x].timeOut)
        {
            killThisThread = true;
        }
        if (killThisThread)
        {
            int y = x;
            while (data[y].threadAlive)
            {
                data[y] = data[y + 1];
                y++;
            }
        }
        else
        {
            data[x].timeTillKill = data[x].timeTillKill - elapsedTime;
            if (data[x].executeIntervall != 0)
            {
                data[x].timeTillNextExecute = data[x].timeTillNextExecute - elapsedTime;
            }
            x++;
        }
    }
    timeStampLastExecute = timeStampThisExecute;
    unsigned long timeTillNextStep = getTimeTillNextStep();
    amountOfExecuteJumps = timeTillNextStep / 4;
    if (amountOfExecuteJumps > 0){
        amountOfExecuteJumps--;
    }
    if (amountOfExecuteJumps > 0)
    {
        OCR1A = 64000;
    }
    else if (timeTillNextStep > 0)
    {
        OCR1A = timeTillNextStep * 16000;
    }
    else
    {
        OCR1A = 16000;
    }
}

int threading::getFirstFreeThread()
{
    int x = 0;
    while (this->data[x].threadAlive)
    {
        x++;
        if (x == MAXIMUM_AMOUNT_OF_THREADS - 1)
        {
            return MAXIMUM_AMOUNT_OF_THREADS - 1;
        }
    }
    return x;
}

unsigned long threading::getNewID()
{
    this->IDcounter++;
    OCR1A = 16000;
    amountOfExecuteJumps = 0;
    return IDcounter;
}

void threading::kill()
{
    processing = true;
    if (IsAlive())
    {
        int x = getThreadByID();
        while (data[x].threadAlive)
        {
            data[x] = data[x + 1];
            x++;
        }
    }
    processing = false;
}

unsigned long threading::getThreadByID()
{
    int x = 0;
    while (x < MAXIMUM_AMOUNT_OF_THREADS)
    {
        if (data[x].ID == ID)
        {
            return x;
        }
        x++;
    }
    return 0;
}

bool threading::IsAlive()
{
    int x = 0;
    while (x < MAXIMUM_AMOUNT_OF_THREADS)
    {
        if (data[x].ID == ID)
        {
            return true;
        }
        x++;
    }
    return false;
}

unsigned long threading::getTimeTillNextStep()
{
    unsigned long timeTillNextStep = 4294967295;
    for (int i = 0; i < MAXIMUM_AMOUNT_OF_THREADS; i++)
    {
        if (data[i].threadAlive)
        {
            if (data[i].executeIntervall < timeTillNextStep)
            {
                timeTillNextStep = data[i].executeIntervall;
            }
        }
        else
        {
            break;
        }
    }
    return timeTillNextStep;
}

void threading::initialize()
{
    if (!hasBeenInitialized)
    {
        for (int i = 0; i < 100; i++)
        {
            data[i].functionPointerVoid = nullptr;
            data[i].threadAlive = false;
        }
    }
    hasBeenInitialized = true;
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Timer Länge brrechnen
    // OCR5A = 62500;            // compare match register 16MHz/256/1Hz
    // 16 = 1 microsekunde
    OCR1A = 16000;          // compare match register 16MHz/256/1Hz
    TCCR1B |= (1 << WGM12); // CTC mode
    // TCCR1B |= (1 << CS10);   // no prescaler
    TCCR1B &= ~(1 << CS10);
    TCCR1B &= ~(1 << CS11);
    TCCR1B &= ~(1 << CS12);
    TCCR1B |= (1 << CS10);   // 1/8 prescaler
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
    interrupts();
}

#endif