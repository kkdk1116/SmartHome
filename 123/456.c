#define F_CPU 16000000 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <util/delay.h>

#define MOTOR_PORT		PORTB       // 스테핑 모터 연결 포트
#define MOTOR_PORT_DDR	DDRB        // 스테핑 모터 연결 포트의 DDR REG

#define MOTOR_ENABLE	(MOTOR_PORT = MOTOR_PORT|0x80)
#define MOTOR_DISABLE	(MOTOR_PORT = MOTOR_PORT&0x7f)
#define MOTOR_STEP_M0	(MOTOR_PORT = (MOTOR_PORT&0xcf)|0x00)
#define MOTOR_STEP_M1	(MOTOR_PORT = (MOTOR_PORT&0xcf)|0x10)
#define MOTOR_STEP_M2	(MOTOR_PORT = (MOTOR_PORT&0xcf)|0x20)
#define MOTOR_STEP_M3	(MOTOR_PORT = (MOTOR_PORT&0xcf)|0x30)
#define MOTOR_LEFT_CLK	(MOTOR_PORT = MOTOR_PORT^0x01)
#define MOTOR_RIGHT_CLK	(MOTOR_PORT = MOTOR_PORT^0x04)
#define MOTOR_LEFT_CW	(MOTOR_PORT = MOTOR_PORT&0xfd)
#define MOTOR_LEFT_CCW	(MOTOR_PORT = MOTOR_PORT|0x02)
#define MOTOR_RIGHT_CW	(MOTOR_PORT = MOTOR_PORT&0xf7)
#define MOTOR_RIGHT_CCW	(MOTOR_PORT = MOTOR_PORT|0x08)
 
//

unsigned int adc_value, light=0;

//

unsigned char pass[4] = {0,0,0,0};
unsigned char passd[] = "1234";
unsigned char cnt = 0;
unsigned int aa=0,bb=-1;
void port_init(void)
{
 PORTA = 0x00;
 DDRA  = 0xFF;
 PORTB = 0x00;
 DDRB  = 0xFF;
 PORTC = 0x00; //m103 output only
 DDRC  = 0x0f;
 PORTD = 0x00;
 DDRD  = 0x00;
 PORTE = 0x00;
 DDRE  = 0x00;
 PORTF = 0x00;
 DDRF  = 0x00;
 PORTG = 0x00;
 DDRG  = 0xFF;

}


void init_devices(void)
{
 //stop errant interrupts until set up
 cli(); //disable all interrupts
 XMCRA = 0x00; //external memory
 XMCRB = 0x00; //external memory
 port_init();

 MCUCR  = 0x00;
 EICRA  = 0x00; //pin change int edge 0:3
 EICRB  = 0x00; //pin change int edge 4:7 //pin change int enable
 EIMSK  = 0x00;

 
 sei(); //re-enable interrupts
 //all peripherals are now initialized
}

void delay(int n)
{
	volatile int i,j;
	for(i=1;i<n;i++)
	{
    	for(j=1;j<600;j++);
	}
}


volatile unsigned char st = '-';
 
void uart0_tx(char d){ while(!(UCSR0A&0x20)); UDR0=d; } 
 
char FONT[] = 
   {  0B00111111, 0B00000110, 0B01011011, 0B01001111, 0B01100110, 
      0B01101101, 0B01111100, 0B00000111, 0B01111111, 0B01100111  } ;

char DIGIT[] = {0xf7, 0xfb, 0xfd, 0xfe} ;
 
ISR(USART0_RX_vect) { 
      st=UDR0;
 } 

void reset_check()
{
	PORTC=0xf0; PORTA=0b01000000;
	PORTG = 0x00;
	cnt=0;
	bb=-1;

}

unsigned char passwd_check(unsigned char pass[])
{
	unsigned char error = 0;
	int i;

	for(i=0;i<4;i++)
	{
		if(pass[i] != passd[i])
			error++;
	}
	
	return error;
}


void passb()
{
	if(cnt != 0 || bb != 0)
	{
		cnt = 0;
		aa = 0;
		bb =-2;
	}
}

void passc(unsigned char data)
{

	if(cnt != 3)
	{
		pass[cnt++] = data;
	}
	else if(cnt == 3)
	{
		pass[cnt]=data;

		if(passwd_check(pass)!=0)
		{
			aa = 0;
			cnt=0;
			bb =-2;
			PORTA=FONT[aa];
		}
		else
		{	
			PORTA = FONT[aa];
			aa=0;
				
			for(int a = 0; a<550; a++){ MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CW; delay(2); MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CW; delay(2);} 
			_delay_ms(2000);
			for(int a = 0; a<550; a++){ MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CCW; delay(2); MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CCW; delay(2);}
			_delay_ms(20);
			cnt = 0;

		}
	}
}

int main() { 
	init_devices();
	MOTOR_PORT_DDR = 0xff;

	int a=0;

	int flag=1;
    DDRD=0xFF; DDRE=0x07;
    ADCSRA=0xE7;

	MOTOR_ENABLE;
	MOTOR_STEP_M3;
   	MOTOR_LEFT_CCW;
   	MOTOR_RIGHT_CCW;

    PORTC=0xf0; PORTA=0b01000000;
    UCSR0B=0x98; UBRR0L=103; //baud rate 9600 
 
     sei(); 
  
     while(1) {
      
		 ADMUX=5; _delay_ms(1); adc_value=ADC;
		 light=(int)((float)adc_value*1000.0/1023.0);
		

	 	 if(light<600)//밝을때
		{	
			MOTOR_ENABLE;
			MOTOR_STEP_M3;
			
			if(flag==1)
			{	
				flag=2;
				for(a=0;a<1150;a++)//while(a<800)
				{	

					MOTOR_LEFT_CW;
					//MOTOR_LEFT_CLK;
					MOTOR_LEFT_CLK;
					delay(2);
					MOTOR_LEFT_CW;
					//MOTOR_LEFT_CLK;
					MOTOR_LEFT_CLK;
					delay(2);
					//a++;
				}
			}
		}
		

		if(light>=600)//어두울때
		{
			MOTOR_ENABLE;
			MOTOR_STEP_M3;

			if(flag==2)
			{
				flag=1;
				for(a=0;a<1100;a++)//while(a<800)
				{	
					MOTOR_LEFT_CCW;
					//MOTOR_LEFT_CLK;
					MOTOR_LEFT_CLK;
					delay(2);
					MOTOR_LEFT_CCW;
					//MOTOR_LEFT_CLK;
					MOTOR_LEFT_CLK;
					delay(2);
					//a++;
				}
			}
		}

		 if ( (st == '1') || (st == '2' ) || (st == '3' ) || (st == '4') || (st == '5' ) || (st == '6')|| (st == '7') || (st == '8') || (st == '9')|| (st == '0') || (st == 'd' ) || (st == 'e' ) ) 
		 {
		 	if(st == '1') {aa = 1; passc('1');}
			else if(st == '2') {aa = 2; passc('2');}
			else if(st == '3') {aa = 3; passc('3');}
			else if(st == '4') {aa = 4; passc('4');}
			else if(st == '5') {aa = 5; passc('5');}
			else if(st == '6') {aa = 6; passc('6');}
			else if(st == '7') {aa = 7; passc('7');}
			else if(st == '8') {aa = 8; passc('8');}
			else if(st == '9') {aa = 9; passc('9');}
			else if(st == '0') {aa = 0; passc('0');}
			else if(st == 'd'){cnt = 0; aa= 0; bb = -2;}
			else if(st == 'e'){ for(int a = 0; a<550; a++){ MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CW; delay(2); MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CW; delay(2);} 
								_delay_ms(20);
								for(int a = 0; a<550; a++){ MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CCW; delay(2); MOTOR_RIGHT_CLK;	MOTOR_RIGHT_CCW; delay(2);}
								_delay_ms(200);
							    
								cnt = 0;}

			if(bb == 3) bb = 0;
			else bb++;

			PORTA = FONT[aa]; 
			PORTC = DIGIT[bb];
			_delay_ms(5);	
		}
		
		

         if ((st == 'o' )|| (st == 'f' ) || (st == 'u' ) || (st == 'b' )|| (st == 'a' ))
		 {
			   if (st == 'o') { PORTD = 0xFF; }
               if (st == 'f') { PORTD = 0x00; }
			   if (st == 'u') { for(a = 0; a<1100; a++){ MOTOR_LEFT_CLK;	MOTOR_LEFT_CCW; delay(2); MOTOR_LEFT_CLK;	MOTOR_LEFT_CCW; delay(2);}}
			   if (st == 'b') {	for(a = 0; a<1150; a++){ MOTOR_LEFT_CLK;	MOTOR_LEFT_CW; delay(2); MOTOR_LEFT_CLK;	MOTOR_LEFT_CW; delay(2);}}
			   if (st == 'a') {  reset_check(); }
		 }	            
          
	  }
	  return 0;
}

