void beep(){
    digitalWrite(PIN_BEEPER,HIGH);
    delay(100);
    digitalWrite(PIN_BEEPER,LOW);
    delay(1000);
}
void doubleBeep(){
    digitalWrite(PIN_BEEPER,HIGH);
    delay(100);
    digitalWrite(PIN_BEEPER,LOW);
    delay(100);
    digitalWrite(PIN_BEEPER,HIGH);
    delay(100);
    digitalWrite(PIN_BEEPER,LOW);
    delay(1000);
}