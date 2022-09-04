void home(){
    stopAxis(0);
    stopAxis(1);
    stopAxis(2);
    if(SIMULATION){
        Serial.println("Willkommen zu Henrys homing routine. Es werden jetzt alle Achsen gehomed. Viel Spaß!!");
        printHomingParameter(X_Axis);
        printHomingParameter(Y_Axis);
        printHomingParameter(Z_Axis);
    }

    X_Axis.homingAbgeschlossen = false;
    Y_Axis.homingAbgeschlossen = false;
    Z_Axis.homingAbgeschlossen = false;

    digitalWrite(X_Axis.Pin.Direction,HIGH);
    digitalWrite(Y_Axis.Pin.Direction,HIGH);
    digitalWrite(Z_Axis.Pin.Direction,HIGH);

    bool homingAllerAchsenAbgeschlossen = false;
    int x = 0;
    unsigned long int myMicros;
    unsigned long int microsLastCycle = micros();
    unsigned long int mycrosSinceLastExecute = 0;
    const unsigned long int intervall = 20*STEPS_PER_MM;
    bool alleAchsenHabenEnstopsVerlassen = false;
    while (!homingAllerAchsenAbgeschlossen){
        myMicros = micros();
        mycrosSinceLastExecute = mycrosSinceLastExecute + (myMicros-microsLastCycle);
        microsLastCycle = myMicros;
        if (mycrosSinceLastExecute > intervall){
            mycrosSinceLastExecute = mycrosSinceLastExecute - intervall;

            if(!alleAchsenHabenEnstopsVerlassen){
                 
                x = 0;
                while (x<3){
                    while(digitalRead(PIN_ENABLE)){microsLastCycle = micros();}
                    Axis = getAxis(x);
                    if(!digitalRead(Axis->Pin.Endstop)){
                        uint8_t AktuellerWertPort = *Axis->Port;
                        if (Axis->toggle){
                            *Axis->Port = AktuellerWertPort |(1<<Axis->StepPinNumber);//Axis->StepPinNumber;//*Axis->Port|(1<<Axis->StepPinNumber);
                            Axis->toggle = false;
                        }else{
                            *Axis->Port = AktuellerWertPort &~(1<<Axis->StepPinNumber);//*Axis->Port|~(0<<Axis->StepPinNumber);
                            Axis->toggle = true;
                        }
                    }
                    x++;                
                }
                if(digitalRead(X_Axis.Pin.Endstop)&digitalRead(Y_Axis.Pin.Endstop)&digitalRead(Z_Axis.Pin.Endstop)){
                    alleAchsenHabenEnstopsVerlassen = true;
                    digitalWrite(X_Axis.Pin.Direction,LOW);
                    digitalWrite(Y_Axis.Pin.Direction,LOW);
                    digitalWrite(Z_Axis.Pin.Direction,LOW);
                    if(SIMULATION){
                        Serial.println("alle Achsen haben den Endstop verlassen");
                    }
                }
            }else{

                while(x < 3){
                    while(digitalRead(PIN_ENABLE)){microsLastCycle = micros();}
                    Axis = getAxis(x);
                    
                    x++;
                    if(!digitalRead(Axis->Pin.Endstop)&!Axis->homingAbgeschlossen){
                        Axis->homingAbgeschlossen = true;
                        Axis->istPosition = 0;
                        if(SIMULATION){
                            if (Axis->AxisNomber == 0){
                                Serial.println("Homing der X Achse abgechlossen! :D");
                            }
                            if (Axis->AxisNomber == 1){
                                Serial.println("Homing der Y Achse abgechlossen! :D");
                            }
                            if (Axis->AxisNomber == 2){
                                Serial.println("Homing der Z Achse abgechlossen! :D");
                            }
                        }
                    
                    }
                    if(!Axis->homingAbgeschlossen){
                        uint8_t AktuellerWertPort = *Axis->Port;
                        if (Axis->toggle){
                            *Axis->Port = AktuellerWertPort |(1<<Axis->StepPinNumber);//Axis->StepPinNumber;//*Axis->Port|(1<<Axis->StepPinNumber);
                            Axis->toggle = false;
                        }else{
                            *Axis->Port = AktuellerWertPort &~(1<<Axis->StepPinNumber);//*Axis->Port|~(0<<Axis->StepPinNumber);
                            Axis->toggle = true;
                        }
                    }
                    if(X_Axis.homingAbgeschlossen & Y_Axis.homingAbgeschlossen & Z_Axis.homingAbgeschlossen){
                        homingAllerAchsenAbgeschlossen = true;
                    }
                    
                }
            }   
        }   
        x = 0;
    }

    digitalWrite(PIN_BEEPER,HIGH);
    delay(200);
    digitalWrite(PIN_BEEPER,LOW);
    delay(200);


    int distanz0PosiBisEndstop = 10;//mm
    move(0,getSteps(distanz0PosiBisEndstop));
    move(1,getSteps(distanz0PosiBisEndstop));
    move(2,getSteps(10));

    while(X_Axis.aktiv || Y_Axis.aktiv || Z_Axis.aktiv){digitalWrite(PIN_BEEPER,HIGH);}
    digitalWrite(PIN_BEEPER,LOW);
    X_Axis.istPosition = 0;
    Y_Axis.istPosition = 0;
    Z_Axis.istPosition = 0;

    if(SIMULATION){
        Serial.println();
        Serial.println("Alle Achsen sind fertig gehomed. Der Hammer!!");
    }
}