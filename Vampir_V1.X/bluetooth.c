#include "bluetooth.h"

//ZA DISPLEJ
void initUART1(void)
{
    U1BRG=0x002f;//ovim odredjujemo baudrate //017f za 1200

    U1MODEbits.ALTIO=1;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne(ALTERNATIVNE)

    IEC0bits.U1RXIE=1;//omogucavamo rx1 interupt

    U1STA&=0xfffc;

    U1MODEbits.UARTEN=1;//ukljucujemo ovaj modul

    U1STAbits.UTXEN=1;//ukljucujemo predaju
}

void WriteUART1(unsigned int data)
{
	  while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}

void WriteUART1dec2string(unsigned int data)
{
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
    WriteUART1(13);
}

void UART1_string(register const char *str)
{
    while((*str) != 0)
    {
        WriteUART1(*str);
        //if(*str == 13) WriteUART1(10);
        //if(*str == 10) WriteUART1(13);
        str++;
    }
    WriteUART1(13); //prelazak u novi red nakon poslatog stringa
}

void initUART2(void)
{
    
    U2BRG=0x002f;; //0x002f;//ovim odredjujemo baudrate //017f za 1200

    //U1MODEbits.ALTIO=0;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne(osnovne)

    IEC1bits.U2RXIE=1;//omogucavamo rx1 interupt

    U2STA&=0xfffc;

    U2MODEbits.UARTEN=1;//ukljucujemo ovaj modul

    U2STAbits.UTXEN=1;//ukljucujemo predaju
   
};

void WriteUART2(unsigned int data)
{
	  while(!U2STAbits.TRMT);

    if(U2MODEbits.PDSEL == 3)
        U2TXREG = data;
    else
        U2TXREG = data & 0xFF;
}

void UART2_string(register const char *str)
{
    while((*str) != 0)
    {
        WriteUART2(*str);
        //if(*str == 13) WriteUART1(10);
        //if(*str == 10) WriteUART1(13);
        str++;
    }
    WriteUART2(13); //prelazak u novi red nakon poslatog stringa
}