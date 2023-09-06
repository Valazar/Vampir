#include <p30fxxxx.h>
#include "tajmer1.h"

#define TMR1_period 100   //1/Fosc * TMR_PERIOD = Interrupt
                            //100us

void initT1(void)
{
	TMR1 = 0;
	PR1 = TMR1_period;
	//PR1 = 0xFFFF;
	T1CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
    //T1CONbits.TCKPS0=1;
    //T1CONbits.TCKPS1=1;
    //T1CONbits.TSYNC=0;
	//IPC1bits. = 0; // T1 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T1IF = 0; // clear interrupt flag
	IEC0bits.T1IE = 1; // enable interrupt

	T1CONbits.TON = 1; // T1 on 
}