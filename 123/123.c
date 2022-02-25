
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#define F_CPU 16000000UL // 16 MHz
#include <util/delay.h>
//
#define LCD_DATA  PORTD
#define LCD_INST  PORTD
#define LCD_CTRL  PORTE
#define LCD_RS    0x01 //PE0
#define LCD_RW    0x02 //PE1
#define LCD_EN    0x04 //PE2
//
unsigned int adc_value1,adc_value2, temp=0, light=0;

//
void LCD_data(char ch){ LCD_DATA=ch; LCD_CTRL=5; _delay_us(1); LCD_CTRL=1; _delay_us(50); }
void LCD_comm(char ch){ LCD_DATA=ch; LCD_CTRL=4; _delay_us(1); LCD_CTRL=0; _delay_ms(5); }
void LCD_CHAR(char c){ LCD_data(c); }
void LCD_STR(char *str){ while(*str)LCD_CHAR(*str++); }
void LCD_pos(char col, char row){ LCD_comm(0x80|(col+row*0x40)); }
void LCD_clear(void){ LCD_comm(1); }
//
void LCD_init(void){
    _delay_ms(100);
    LCD_comm(0x38); LCD_comm(0x38); LCD_comm(0x38);
    LCD_comm(0x0E); LCD_comm(0x06); LCD_clear();
}

//
int main(void){




    char str[20]="LCD test..      ";
    DDRD=0xFF; DDRE=0x07;
    LCD_init();
    ADCSRA=0xE7;

    while(1)
	{

        _delay_ms(250);


        adc_value1=ADC;
		adc_value2=ADC;

		ADMUX=0x43; _delay_ms(1); adc_value1=ADC;
        temp=(int)( 2*(float)adc_value1*1024.0/1024.0); // LM358 없는 경우 5000.0으로 수정
        //if(temp>400)PORTB=1; else PORTB=0; // 40도 이상이면 부저 ON
        LCD_pos(0,0); sprintf(str,"Temp=%3d.%d",temp/10,temp%10); LCD_STR(str);


		ADMUX=0x45; _delay_ms(1); adc_value2=ADC;
		light=(int)((float)adc_value2*1000.0/1024.0);
		ADMUX=0x46; _delay_ms(1); adc_value2=ADC;
		LCD_pos(0,1); sprintf(str,"Light=%d",light); LCD_STR(str);

	}	
}

