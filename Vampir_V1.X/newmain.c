#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include <outcompare.h>
#include "adc.h"
#include "bluetooth.h"
#include "tajmer1.h"
#include "timer2.h"
#include "fotootpornik.h"
#include <string.h>

#define BUFFER_SIZE 6

_FOSC(CSW_FSCM_OFF & FRC_PLL4);//instruction takt je isti kao i interni oscilator
_FWDT(WDT_OFF);

unsigned int br1, br2;
unsigned int k,r1,r2;
int startFlag = 0;
int stopFlag = 0;
unsigned char tempRX;   //RS232
unsigned char buffer[BUFFER_SIZE];
unsigned char target1[BUFFER_SIZE] = "START";
unsigned char target2[BUFFER_SIZE] = "STOP";
unsigned char n;

unsigned int sirovi0, sirovi1, sirovi2, sirovi3;   //AD konverzija
unsigned int brojac_ms,stoperica,stoperica_ms,sekund; //Tajmer
unsigned int temp0, temp1, temp2, temp3;//fotootpronici


void __attribute__((__interrupt__)) _ADCInterrupt(void) {						
	sirovi0 = ADCBUF0;
    sirovi1 = ADCBUF1;
    sirovi2 = ADCBUF2;
    sirovi3 = ADCBUF3;
    IFS0bits.ADIF = 0;
} 

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    tempRX = U1RXREG;
}

/*void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void){
    
    //tempRX = U2RXREG;
    IFS1bits.U2RXIF = 0;
    buffer[n] = U2RXREG;
	if(n < BUFFER_SIZE)	n++;
	else n = 0;

}*/

void __attribute__ ((__interrupt__)) _T1Interrupt(void) { // svakih 100us
    TMR1 =0;
    
    stoperica++;    //brojac za funkciju delay_10us()
    
    if(stoperica%100 == 0)
        stoperica_ms++; //brojac za funkciju delay_ms()

	IFS0bits.T1IF = 0; 
}

void __attribute__((__interrupt__)) _T2Interrupt(void){
   	TMR2 =0;
    IFS0bits.T2IF = 0;
}

void flushU2RX(void){
    char dummyData;
    while (U2STAbits.URXDA) // wait for the receive buffer to be empty
    {
        dummyData = U2RXREG; // read the received data and discard it
    }
}

void initPins(){
    //led blink
    TRISDbits.TRISD8=0;
    
    TRISBbits.TRISB0=1; //FOTOOTPORNICI
    TRISBbits.TRISB1=1;
    TRISBbits.TRISB2=1;
    TRISBbits.TRISB3=1;
    
    ADPCFGbits.PCFG0=0; // AD KONVERZIJA FOTOOTPORNIKA
    ADPCFGbits.PCFG1=0; //analogni pinovi
    ADPCFGbits.PCFG2=0; 
    ADPCFGbits.PCFG3=0;
    
    //ULTRAZVUCNI NAPRIJED
    TRISBbits.TRISB4=0; //trigger
    TRISBbits.TRISB5=1; //echo
    
    //ULTRAZVUCNI NAZAD
    TRISBbits.TRISB10=0; //triger
    TRISBbits.TRISB9=1; //echo
    
    ADPCFGbits.PCFG9=1; // digitalni pinovi
    ADPCFGbits.PCFG10=1;
    ADPCFGbits.PCFG4=1; 
    ADPCFGbits.PCFG5=1;
    
    //pinovi za kretanje
    TRISFbits.TRISF0=0;   //in1 
    TRISFbits.TRISF1=0;   //in2 
    TRISFbits.TRISF2=0;   //in4
    TRISFbits.TRISF3=0;   //in3
    
    //pwm 
    TRISDbits.TRISD1=0;    //enB
    TRISDbits.TRISD2=0;    //enA
    
    
}

void initPWM(){
    
        PR2=2499;//odredjuje frekvenciju po formuli
        OC1RS=20;//postavimo pwm
        OC1R=1000;//inicijalni pwm pri paljenju samo
        OC1CON =OC_IDLE_CON & OC_TIMER2_SRC & OC_PWM_FAULT_PIN_DISABLE& T2_PS_1_256;//konfiguracija pwma
        //T2CONbits.TON=1;//ukljucujemo timer koji koristi

        //for(br1=0;br1<700;br1++)
        //for(br2=0;br2<3000;br2++);
        
        OC3RS=20;//postavimo pwm
        OC3R=1000;//inicijalni pwm pri paljenju samo
        OC3CON =OC_IDLE_CON & OC_TIMER2_SRC & OC_PWM_FAULT_PIN_DISABLE& T2_PS_1_256;//konfiguracija pwma
        T2CONbits.TON=1;//ukljucujemo timer koji koristi

        //for(br1=0;br1<700;br1++)
        //for(br2=0;br2<3000;br2++);

}

void delay_10us(int vreme){
    stoperica = 0;
	while(stoperica < vreme);
}

void delay_ms(int v){
    stoperica_ms = 0;
    while(stoperica_ms < v);
}

double rastojanje_D1();
double rastojanje_D2();

int i;
int rNaprijed = 0, rNazad = 0;
int broj1, broj2;
int fleg = 0;

// Function to process received data
void processData(const char* data) {
    // Check if received data is "START"
    if (strcmp(data, "START") == 0) {
        // Set startFlag to 1
        startFlag = 1;
        //WriteUART2(startFlag);
        
    }
    // Check if received data is "STOP"
    else if (strcmp(data, "STOP") == 0) {
        // Set stopFlag to 1
        stopFlag = 1;
        //WriteUART2(stopFlag);
    }
   
}

// Function to receive data via Bluetooth
void receiveData() {
    static char buffer[BUFFER_SIZE];
    static int bufferIndex = 0;
    char receivedChar;
    
    // Check if UART receiver buffer has data available
    if (U2STAbits.URXDA) {
        // Read the received character
        receivedChar = U2RXREG;
        
        // Check if the received character is a newline or carriage return
        if (receivedChar == '\n' || receivedChar == '\r') {
            // Null-terminate the buffer
            buffer[bufferIndex] = '\0';
            
            // Process the received data
            processData(buffer);
            
            // Reset buffer index for the next reception
            bufferIndex = 0;
        }
        // Check if the buffer is not full
        else if (bufferIndex < BUFFER_SIZE - 1) {
            // Add the received character to the buffer
            buffer[bufferIndex] = receivedChar;
            bufferIndex++;
        }
    }
}

int main(void) {
    
    initPins();
    initADC();
    //initUART1();    //Inicijalizacija za komunikacije
    initUART2();    //prisluskivanje za blutut
    initT1();
    initPWM();
    
    ADCON1bits.ADON=1;  //Pocetak AD konverzije
    
    zaustaviSve();  //Inicijalizacija kretanja auta
    
    while(1){   
       
        temp0=fotootpornik(sirovi0); //privremene promjenjive za svaki fotootp.
        temp1=fotootpornik(sirovi1); //pokazuju da li je on osvjetljen
        temp2=fotootpornik(sirovi2);
        temp3=fotootpornik(sirovi3);
        
        /*delay_ms(500);
        LATDbits.LATD8=~LATDbits.LATD8;
        delay_ms(500);*/
        
        receiveData(); //funckija za primanje podataka    
        rNazad = rastojanje_D2();    // za nazad zaustavi se na r=6 
        rNaprijed = rastojanje_D1(); // za naprijed zaustavi se na r=9*/  
        
        
        //if(startFlag==1 && stopFlag==0 ) {
            if(temp0 == 1){
                UART2_string("------Kretanje naprijed------");
                idiNaprijed();
                if(rNaprijed < 9 ){
                    zaustaviSve();
                }
            }
            else if(temp1 == 1){
                UART2_string("------Kretanje lijevo------");
                idiLijevo();
                delay_ms(500);
                zaustaviSve();
            }
                
            else if(temp2 == 1){
                 UART2_string("------Kretanje desno------");
                 idiDesno();
                 delay_ms(500);
                 zaustaviSve();
            }
            else if(temp3 == 1){
                UART2_string("------Kretanje nazad------");
                idiNazad();
                if(rNazad < 9){
                    zaustaviSve();
                }
            }
         //startFlag=0;   
       /* }
        else if(stopFlag==1){
           zaustaviSve();
           stopFlag=0;
        }*/
        
       
    }
    return 0; 
}
  

double rastojanje_D1() //RASTOJANJE NAPRIJED 
{
    int time1 = 0;
    double range_D1 = 0;
    fleg = 0;
    LATBbits.LATB10 = 1;
    delay_10us(5);
    LATBbits.LATB10 = 0;

    while(!PORTBbits.RB9);
    
    while(PORTBbits.RB9)
    {
        time1++;
        delay_10us(1);
        if(time1 == 500)
            fleg = 1;
    };
    
    range_D1 = time1 * 0.17;
    return range_D1;
}

double rastojanje_D2() //RASTOJANJE NAZAD
{
    int time2 = 0;
    double range_D2 = 0;
    LATBbits.LATB4 = 1;
    delay_10us(5);
    LATBbits.LATB4 = 0;
    
    while(!PORTBbits.RB5);

    while(PORTBbits.RB5)
    {
        time2++;
        delay_10us(1);
    };
    
    range_D2 = time2 * 0.17;
    return range_D2;
}

void idiNaprijed() 
{
    //UART1_string("naprijed");
        OC1RS=1800;
        OC3RS=1800;
    LATFbits.LATF0=0;  
    LATFbits.LATF1=1;   
    LATFbits.LATF2=1;
    LATFbits.LATF3=0;

}

void idiLijevo() 
{
    //UART1_string("Lijevo");
        OC1RS=1800;
        OC3RS=1800;      
    LATFbits.LATF0=0;   //in1  
    LATFbits.LATF1=1;   //in2 
    LATFbits.LATF2=0;   //in3
    LATFbits.LATF3=1;   //in4
}

void idiDesno() 
{
    //UART1_string("desno");
    OC1RS=1800;
    OC3RS=1800;
    LATFbits.LATF0=1;   //in1 
    LATFbits.LATF1=0;   //in2 
    LATFbits.LATF2=1;   //in3
    LATFbits.LATF3=0;   //in4
}

void idiNazad() 
{
    //UART1_string("Nazad");
        OC1RS=1800;
        OC3RS=1800; 
    LATFbits.LATF0=1;  
    LATFbits.LATF1=0;   
    LATFbits.LATF2=0;
    LATFbits.LATF3=1;
}

void zaustaviSve()
{
   //UART1_string("stop");
   OC1RS=0;
   OC3RS=0;
   LATFbits.LATF0=0;  
   LATFbits.LATF1=0;   
   LATFbits.LATF2=0;
   LATFbits.LATF3=0; 
    
}

