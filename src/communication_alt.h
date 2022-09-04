const int protocollLength = 7;
unsigned long int cyclesSinceBufferWasEmpty;
unsigned long int cyclesSinceBufferWasFull;


unsigned int TwoBytesToMM(byte Byte1, byte Byte2, unsigned long int maxPosition){
    unsigned int maxPosition_mm = getMillimeter(maxPosition);
    unsigned long int ValueTwoBytes = Byte1;
    ValueTwoBytes = ValueTwoBytes*256;
    ValueTwoBytes = ValueTwoBytes + Byte2;
    ValueTwoBytes = ValueTwoBytes * maxPosition_mm-1;
    ValueTwoBytes = ValueTwoBytes / 65535;
    return ValueTwoBytes;
}

unsigned long int TwoBytesToSteps(byte Byte1, byte Byte2, unsigned long int maxPosition){
    unsigned long int ValueTwoBytes = Byte1;
    ValueTwoBytes = ValueTwoBytes*256;
    ValueTwoBytes = ValueTwoBytes + Byte2;
    ValueTwoBytes = ValueTwoBytes * (maxPosition-1);
    ValueTwoBytes = ValueTwoBytes / 65535;
    if (ValueTwoBytes > maxPosition-1){ValueTwoBytes = maxPosition-1;}
    return ValueTwoBytes;
}

void clearBuffer(){
    while(Serial.available() > 0) {
        char t = Serial.read();
    }
    cyclesSinceBufferWasEmpty = 0;
    cyclesSinceBufferWasFull = 0;
    Serial.write(255);
    
}
bool toggle;
bool getCommand(){
    
    if (Serial.available() == protocollLength){
        int buffer[7];
        int x = 0;
        while (x<protocollLength){
            buffer[x] = Serial.read();
            if (buffer[0] != 0){
                clearBuffer();
                return false;
            }
            if (buffer[x]<0 || buffer[x]>255){
                clearBuffer();
                Serial.println(buffer[x]);
                return false;
            }
            x++;
        }

        if(Serial.available() == 0){
            newCommand.X_Position = TwoBytesToSteps(buffer[1],buffer[2],X_Axis.MaxPosition);
            newCommand.Y_Position = TwoBytesToSteps(buffer[3],buffer[4],Y_Axis.MaxPosition);
            newCommand.Z_Position = TwoBytesToSteps(buffer[5],buffer[6],Z_Axis.MaxPosition);
            if (mixAxis){
                unsigned long int HalbesZ = Z_Axis.MaxPosition / 2;
                unsigned long int OffsetZ = 0;
                bool OffsetNegativ = false;
                if(newCommand.Z_Position > HalbesZ){
                    OffsetZ = newCommand.Z_Position - HalbesZ;
                    OffsetNegativ = true;
                }
                if(newCommand.Z_Position < HalbesZ){
                    OffsetZ = HalbesZ - newCommand.Z_Position;
                }

                if(OffsetNegativ){
                    HalbesZ = HalbesZ * 0.6;
                }


                if (newCommand.Z_Position < HalbesZ){
                    newCommand.Z_Position = HalbesZ - newCommand.Z_Position;
                    newCommand.Z_Position = newCommand.Z_Position * 1.5;
                    if (newCommand.Z_Position > HalbesZ){
                        newCommand.Z_Position = HalbesZ;
                    }
                    newCommand.Z_Position = HalbesZ - newCommand.Z_Position;
                }

                if (!OffsetNegativ){
                    newCommand.X_Position = newCommand.X_Position + OffsetZ;
                    if(newCommand.X_Position > X_Axis.MaxPosition-1){
                        newCommand.X_Position = X_Axis.MaxPosition-1;
                    }
                }

                if (!OffsetNegativ){
                    newCommand.Y_Position = newCommand.Y_Position + OffsetZ;
                    if(newCommand.Y_Position > Y_Axis.MaxPosition-1){
                        newCommand.Y_Position = Y_Axis.MaxPosition-1;
                    }
                }

                if (OffsetNegativ){
                    if(newCommand.X_Position > OffsetZ){
                        newCommand.X_Position = newCommand.X_Position - OffsetZ;
                    }else{
                        newCommand.X_Position = 0;
                    }
                }

                if (OffsetNegativ){
                    if(newCommand.Y_Position > OffsetZ){
                        newCommand.Y_Position = newCommand.Y_Position - OffsetZ;
                    }else{
                        newCommand.Y_Position = 0;
                    }
                }
            }



            //Serial.write(newCommand.X_Position/(X_Axis.MaxPosition/100));
            //Serial.write(newCommand.Y_Position/(Y_Axis.MaxPosition/100));
            //Serial.write(newCommand.Z_Position/(Z_Axis.MaxPosition/100));
            clearBuffer();
            return true;
        }
    }
    if (Serial.available() != 0){
        cyclesSinceBufferWasEmpty++;
        cyclesSinceBufferWasFull = 0;
    }else{
        cyclesSinceBufferWasFull++;
        cyclesSinceBufferWasEmpty = 0;
    }
    if (cyclesSinceBufferWasEmpty == 3000){
        Serial.write(Serial.available());
        if (Serial.available() > protocollLength){
            digitalWrite(beeperPin,HIGH);
            delay(100);
            digitalWrite(beeperPin,LOW);
        }else{
            digitalWrite(beeperPin,HIGH);
            delay(100);
            digitalWrite(beeperPin,LOW);
            delay(100);
            digitalWrite(beeperPin,HIGH);
            delay(100);
            digitalWrite(beeperPin,LOW);
        }
        clearBuffer();
        return false;
    }

    if (cyclesSinceBufferWasFull == 30000){
        digitalWrite(beeperPin,HIGH);
        delay(100);
        digitalWrite(beeperPin,LOW);
        clearBuffer();
        return false;
    }
    return false;
}





