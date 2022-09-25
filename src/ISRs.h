ISR(TIMER1_COMPA_vect){recalculateAccelleration();}
ISR(TIMER3_COMPA_vect){Axis = getAxis(0);newStep();}
ISR(TIMER4_COMPA_vect){Axis = getAxis(1);newStep();}
ISR(TIMER5_COMPA_vect){Axis = getAxis(2);newStep();}