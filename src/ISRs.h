ISR(TIMER1_COMPA_vect){recalculateAccelleration();}
ISR(TIMER3_COMPA_vect){Axis = getAxis(0);newStep();executed_X = true;}
ISR(TIMER4_COMPA_vect){Axis = getAxis(1);newStep();executed_Y = true;}
ISR(TIMER5_COMPA_vect){Axis = getAxis(2);newStep();executed_Z = true;}