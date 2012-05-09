/* file main.c */

By
Gharat paurva
Kulkarni Jyoti
Patil Supriya
****************************************************************************************

LCD Connections:
 			 LCD	  Microcontroller Pins
 			  RS  --> PC0
			  RW  --> PC1
			  EN  --> PC2
			  DB7 --> PC7
			  DB6 --> PC6
			  DB5 --> PC5
			  DB4 --> PC4

 Hardware Setup: Refer Table 3.8 from chapter 3 for LCD connection. Connect the 10 pin  FRC cable between 
                 PORTC and the LCD Header.

 Refer product manual for more detailed description.s

 Note: 
 
 1. Make sure that in the configuration options following settings are 
 done for proper operation of the code

 Microcontroller: atmega640
 Frequency: 14745600
 Optimization:-O0 

************************************************************************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define RS 0
#define RW 1
#define EN 2
#define lcd_port PORTC

#define sbit(reg,bit)	reg |= (1<<bit)			// Macro defined for Setting a bit of any register.
#define cbit(reg,bit)	reg &= ~(1<<bit)		// Macro defined for Clearing a bit of any register.

void init_ports();
void lcd_reset();
void lcd_init();
void lcd_wr_command(unsigned char);
void lcd_wr_char(char);
void lcd_line1();
void lcd_line2();
void lcd_string(char*);

//unsigned int temp;
unsigned int unit;
unsigned int tens;
unsigned int hundred;
unsigned int thousand;
unsigned int million;
unsigned char PORTA_count = 0;
unsigned char temp;

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
char first_row[] = " POT1 POT2 POT3 ";


//auto
//unsigned char PORTA_count = 0;
unsigned char temp;
int count=0;
unsigned char set_number = 0;           // selecting the ser of servo		
unsigned char angle_upper_byte = 0; 	// temprory storing upper byte of desired angle value 
unsigned char angle_lower_byte = 0; 	// temprory storing lower byte of desired angle value 
unsigned char degree = 0;                           
// final storing of temprory variable for individual servo as upper & lower bytes
unsigned char angle_upper_byte_servo2, angle_lower_byte_servo2, 
              angle_upper_byte_servo3, angle_lower_byte_servo3, 
			  angle_upper_byte_servo4, angle_lower_byte_servo4;
unsigned char angle_upper_byte_servo5, angle_lower_byte_servo5, 
			  angle_upper_byte_servo6, angle_lower_byte_servo6, 
			  angle_upper_byte_servo7, angle_lower_byte_servo7;
//--------------------------------------------------------------------------------
//port initialisation for Servos i/p's												
//--------------------------------------------------------------------------------																			
void servo_pin_config (void)
{ 
 DDRE  = 0xFE;    //PL 1,2,3,4,5,6,7 as output
 PORTE = 0x00;
}

//--------------------------------------------------------------------------------
// reset & set function of servos
//--------------------------------------------------------------------------------
void reset_servo2 (void)
{ PORTE = PORTE & 0xFD; }

void set_servo2 (void)
{ PORTE = PORTE | 0x02; }

void reset_servo3 (void)
{ PORTE = PORTE & 0xFB; }

void set_servo3 (void)
{ PORTE = PORTE | 0x04; }

void reset_servo4 (void)
{ PORTE = PORTE & 0xF7; }

void set_servo4 (void)
{ PORTE = PORTE | 0x08; }

void reset_servo5 (void)
{ PORTE = PORTE & 0xEF; }

void set_servo5 (void)
{ PORTE = PORTE | 0x10; }

void reset_servo6 (void)
{ PORTE = PORTE & 0xDF; }

void set_servo6 (void)
{ PORTE = PORTE | 0x20; }

void reset_servo7 (void)
{ PORTE = PORTE & 0xBF; }

void set_servo7 (void)
{ PORTE = PORTE | 0x40; }

//--------------------------------------------------------------------------------
// TIMER1 initialize - prescale:1
// WGM: 0) Normal, TOP=0xFFFF
// desired value: 400Hz
// actual value: 400.007Hz (0.0%)
//--------------------------------------------------------------------------------
void timer1_init(void)
{
 TCCR1B = 0x00; //stop
 TCNT1H = 0x70; //setup
 TCNT1L = 0x01;

 /* In Use */
 OCR1AH = 0x8F;
 OCR1AL = 0xFF;
 
 OCR1BH = 0x8F;
 OCR1BL = 0xFF;
 OCR1CH = 0x00;
 OCR1CL = 0x00;
 ICR1H  = 0x8F;
 ICR1L  = 0xFF;
 TCCR1A = 0x00;
 TCCR1C = 0x00;
 TCCR1B = 0x01; //start Timer
}


//--------------------------------------------------------------------------------
// timer1 comparatorA match with timer register ISR,
// This ISR used for reset servo on set number 
//--------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
 //compare occured TCNT1=OCR1A
 if (set_number == 0) { reset_servo2(); }
 if (set_number == 1) { reset_servo3(); }
}

//--------------------------------------------------------------------------------
// timer1 comparatorB match with timer register ISR,
// This ISR used for reset servo set number
//--------------------------------------------------------------------------------
ISR(TIMER1_COMPB_vect)
{
 //compare occured TCNT1=OCR1B
 if (set_number == 0) { reset_servo4(); }
 if (set_number == 1) { reset_servo5(); }
}

//--------------------------------------------------------------------------------
// timer1 comparatorC match with timer register ISR,
// This ISR used for reset servo set number
//--------------------------------------------------------------------------------
ISR(TIMER1_COMPC_vect)
{
 //compare occured TCNT1=OCR1c
 if (set_number == 0) { reset_servo6(); }
 if (set_number == 1) { reset_servo7(); }
}

//--------------------------------------------------------------------------------
// timer1 overflow ISR,
// This ISR can be used to load the PWM value. Here each Servo motor is
// move between 0 to 180 degrees proportional to the pulse ON time between 
// 0.5 to 2.2 ms with the frequency between 40 to 60 Hz. ie. 400Hz/8 = 50Hz
//--------------------------------------------------------------------------------
ISR(TIMER1_OVF_vect)
{
 //TIMER1 has overflowed
 TCNT1H = 0x70; //reload counter high value	for 400Hz	
 TCNT1L = 0x01; //reload counter low value for 400Hz

 set_number ++;

 if (set_number>7)     				// 400Hz/8 = 50Hz
 {
  	set_number = 0;
 }

 if (set_number == 0)               // 1st set of servo motor
 {
  set_servo2(); 
  set_servo4(); 
  set_servo6();
  OCR1AH = angle_upper_byte_servo2;
  OCR1AL = angle_lower_byte_servo2;
  OCR1BH = angle_upper_byte_servo4;
  OCR1BL = angle_lower_byte_servo4;
  OCR1CH = angle_upper_byte_servo6;
  OCR1CL = angle_lower_byte_servo6;
 }
 
 if (set_number == 1)               // 2nd set of servo motor   
 {
  set_servo3(); 
  set_servo5(); 
  set_servo7();
  OCR1AH = angle_upper_byte_servo3;
  OCR1AL = angle_lower_byte_servo3;
  OCR1BH = angle_upper_byte_servo5;
  OCR1BL = angle_lower_byte_servo5;
  OCR1CH = angle_upper_byte_servo7;
  OCR1CL = angle_lower_byte_servo7;
 }

}

//--------------------------------------------------------------------------------
// function for angular movement calculation
//--------------------------------------------------------------------------------
void angle_value_calculation (void)
{
 unsigned int angle_value = 0;
 unsigned int temp = 0;
 if (degree > 180)
 degree = 180; // limiting the scope of the servo rotation
 
 angle_value = 0x8FAE + (139 * (unsigned char) degree); //actual constant is 139.4
 angle_lower_byte = (unsigned char) angle_value;        //separating the lower byte
 
 temp = angle_value >> 8;
 angle_upper_byte = (unsigned char) temp;               //separating the upper byte
}

//---------------------.-----------------------------------------------------------
// Function for individual servo angle calculation call with degree as parameter & 
// store into respective variables. 
//--------------------------------------------------------------------------------

void angle_servo2 (unsigned char angle)                // for servo2 
{
 degree = angle;
 angle_value_calculation();
 angle_upper_byte_servo2 = angle_upper_byte;
 angle_lower_byte_servo2 = angle_lower_byte;
}

/* In Use */
void angle_servo3 (unsigned char angle)                // for servo3 
{
 degree = angle;
 angle_value_calculation();
 angle_upper_byte_servo3 = angle_upper_byte;
 angle_lower_byte_servo3 = angle_lower_byte;
}

/* In Use */
void angle_servo4 (unsigned char angle)                // for servo4 
{
 degree = angle;
 angle_value_calculation();
 angle_upper_byte_servo4 = angle_upper_byte;
 angle_lower_byte_servo4 = angle_lower_byte;
}

void angle_servo5 (unsigned char angle)                // for servo5 
{
 degree = angle;
 angle_value_calculation();
 angle_upper_byte_servo5 = angle_upper_byte;
 angle_lower_byte_servo5 = angle_lower_byte;
}

void angle_servo6 (unsigned char angle)                // for servo6 
{
 degree = angle;
 angle_value_calculation();
 angle_upper_byte_servo6 = angle_upper_byte;
 angle_lower_byte_servo6 = angle_lower_byte;
}

void angle_servo7 (unsigned char angle)                // for servo7
{
 degree = angle;
 angle_value_calculation();
 angle_upper_byte_servo7 = angle_upper_byte;
 angle_lower_byte_servo7 = angle_lower_byte;
}


void switchs_config (void)
{
 DDRA = DDRA & 0x00;   //PORTE pin set as input  
 PORTA = PORTA | 0xFF; //PORTE internal pull-up enabled
}

//Function to configure LCD port
void lcd_port_config (void)
{
 DDRC = DDRC | 0xF7; //all the LCD pin's direction set as output
 PORTC = PORTC & 0x80; // all the LCD pins are set to logic 0 except PORTC 7
}


//Function to Reset LCD
void lcd_set_4bit()
{
	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x30;				//Sending 3
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//Delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x30;				//Sending 3
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//Delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x30;				//Sending 3
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//Delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	_delay_ms(1);

	cbit(lcd_port,RS);				//RS=0 --- Command Input
	cbit(lcd_port,RW);				//RW=0 --- Writing to LCD
	lcd_port = 0x20;				//Sending 2 to initialise LCD 4-bit mode
	sbit(lcd_port,EN);				//Set Enable Pin
	_delay_ms(5);					//Delay
	cbit(lcd_port,EN);				//Clear Enable Pin

	
}

//Function to Initialize LCD
void lcd_init()
{
	_delay_ms(1);

	lcd_wr_command(0x28);			//LCD 4-bit mode and 2 lines.
	lcd_wr_command(0x01);
	lcd_wr_command(0x06);
	lcd_wr_command(0x0E);
	lcd_wr_command(0x80);
		
}

	 
//Function to Write Command on LCD
void lcd_wr_command(unsigned char cmd)
{
	unsigned char temp;
	temp = cmd;
	temp = temp & 0xF0;
	lcd_port &= 0x0F;
	lcd_port |= temp;
	cbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);
	
	cmd = cmd & 0x0F;
	cmd = cmd<<4;
	lcd_port &= 0x0F;
	lcd_port |= cmd;
	cbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);
}

//Function to Write Data on LCD
void lcd_wr_char(char letter)
{
	char temp;
	temp = letter;
	temp = (temp & 0xF0);
	lcd_port &= 0x0F;
	lcd_port |= temp;
	sbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);

	letter = letter & 0x0F;
	letter = letter<<4;
	lcd_port &= 0x0F;
	lcd_port |= letter;
	sbit(lcd_port,RS);
	cbit(lcd_port,RW);
	sbit(lcd_port,EN);
	_delay_ms(5);
	cbit(lcd_port,EN);
}


//Function to bring cursor at home position
void lcd_home()
{
	lcd_wr_command(0x80);
}


//Function to Print String on LCD
void lcd_string(char *str)
{
	while(*str != '\0')
	{
		lcd_wr_char(*str);
		str++;
	}
}

//Position the LCD cursor at "row", "column".

void lcd_cursor (char row, char column)
{
	switch (row) {
		case 1: lcd_wr_command (0x80 + column - 1); break;
		case 2: lcd_wr_command (0xc0 + column - 1); break;
		case 3: lcd_wr_command (0x94 + column - 1); break;
		case 4: lcd_wr_command (0xd4 + column - 1); break;
		default: break;
	}
}

//Function To Print Any input value upto the desired digit on LCD
void lcd_print (char row, char coloumn, unsigned int value, int digits)
{
	unsigned char flag=0;
	if(row==0||coloumn==0)
	{
		lcd_home();
	}
	else
	{
		lcd_cursor(row,coloumn);
	}
	if(digits==5 || flag==1)
	{
		million=value/10000+48;
		lcd_wr_char(million);
		flag=1;
	}
	if(digits==4 || flag==1)
	{
		temp = value/1000;
		thousand = temp%10 + 48;
		lcd_wr_char(thousand);
		flag=1;
	}
	if(digits==3 || flag==1)
	{
		temp = value/100;
		hundred = temp%10 + 48;
		lcd_wr_char(hundred);
		flag=1;
	}
	if(digits==2 || flag==1)
	{
		temp = value/10;
		tens = temp%10 + 48;
		lcd_wr_char(tens);
		flag=1;
	}
	if(digits==1 || flag==1)
	{
		unit = value%10 + 48;
		lcd_wr_char(unit);
	}
	if(digits>5)
	{
		lcd_wr_char('E');
	}
	
}



//This Function accepts the Channel Number and returns the corresponding Analog Value 
unsigned char ADC_Conversion(unsigned char Ch)
{
 unsigned char a;

 Ch = Ch & 0x07;  			
 ADMUX= 0x20| Ch;	   		
 ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
 while((ADCSRA&0x10)==0);	    //Wait for ADC conversion to complete
 a = ADCH;
 ADCSRA = ADCSRA|0x10;          //clear ADIF (ADC Interrupt Flag) by writing 1 to it
 ADCSRB = 0x00;
 return a;
}

void print_sensor(char row, char coloumn,unsigned char channel)
{
 ADC_Value = ADC_Conversion(channel);
 lcd_print(row, coloumn, ADC_Value, 3);
}


//Function to Initialize ADC
void adc_init()
{
 ADCSRA = 0x00;
 ADCSRB = 0x00;		//MUX5 = 0
 ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
 ACSR = 0x80;
 ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//UART0 initialize
// desired baud rate:115200
// actual baud rate:115200 (0.0%)
// char size: 8 bit
// parity: Disabled

void uart0_init(void)
{
 UCSR0B = 0x00; //disable while setting baud rate
 UCSR0A = 0x00;
 UCSR0C = 0x06;
 UBRR0L = 0x07; //set baud rate lo
 UBRR0H = 0x00; //set baud rate hi
 UCSR0B = 0x98;
}


SIGNAL(SIG_USART0_RECV) 		// ISR for receive complete interrupt
{
 unsigned char data;

	data = UDR0; 	//making copy of data from UDR0 in 'data' variable 
    	if(data == 0x38) 				//ascii VALUE FOR 8
	{
		angle_servo4(110);
 		_delay_ms(2000);

  		angle_servo4(90);
 		_delay_ms(2000);	
	}
}

//--------------------------------------------------------------------------------
//call this routine to initialize all peripherals
//--------------------------------------------------------------------------------
void init_devices(void)
{
 			//stop errant interrupts until set up
 cli();                                	// disable all interrupts

 servo_pin_config();                   	// servo configuration 
 timer1_init(); 
 
 /* In Use */
 angle_servo3(80);//outer
 angle_servo4(90);//inner
 
 adc_init();
 port_init();				
			// initilize timer1
 TIMSK1 = 0x0F;                          // timer1 interrupt sources
			// timer1 interrupt sources
 sei(); //re-enable interrupts
			 //all peripherals are now initialized
 uart0_init();		 //to Rx data from PC to MCU through serial comm
}

//Function to Initialize PORTS
void port_init()
{
	lcd_port_config();
	switchs_config();
	
}


//Main Function
void main(void)
	{
	unsigned long int avg,MODE=0;
	unsigned char i=0;	
	

	init_devices();  
	angle_servo3(80);//outer
	angle_servo4(90);//inner	
	lcd_set_4bit();
	lcd_init();
	lcd_cursor(1,1);
	lcd_string("switch Demo");

	_delay_ms(1000);

	while(1)
		{
		temp=PINA;
		if((temp & 0x01) == 0x00)         
       		{ 
		//	display_clear();  
			lcd_cursor(2,1);
			lcd_string("switch 1 Auto");
			MODE=1;
			//status = 0x00;
			}
		if((temp & 0x02) == 0x00)         
        	{ 
		//	display_clear();	  
			lcd_cursor(2,1);
			lcd_string("switch 2 Accelo");
			//staus=0x01;
			MODE=2;
			}

		if(MODE==1)
			{
			//initilise all servo with zero degree calibration
			angle_servo2(90);
			angle_servo3(80);//outer
			angle_servo4(90);//inner
			angle_servo5(0);
			angle_servo6(0);
			angle_servo7(0);
			_delay_ms(1000);       // delay of 6sec

			
       			 UDR0 = '8';           // this will send '8' after every 5 seconds


 		 	for(i=0;i<2;i++)
				{
				 angle_servo4(110);
				 _delay_ms(500);
				  angle_servo4(90);
				 _delay_ms(500);
				 angle_servo3(60);
				 _delay_ms(500);
				 angle_servo3(80);
				 _delay_ms(500);
				  angle_servo4(110);
				 _delay_ms(500);			
			 	}
				angle_servo4(90);
				_delay_ms(500);
			}		
		
		if(MODE==2)
			{
			 avg=ADC_Conversion(0)-10;
		//	 lcd_print(2, 2, avg, 3);//Prints value of POT1
			 angle_servo3(80-(avg-80));

			 avg=ADC_Conversion(1);
	//		 lcd_print(2, 7, avg, 3);//Prints value of POT2
			 angle_servo4(avg);
 			}
 			 

		
	    }
	
	}


