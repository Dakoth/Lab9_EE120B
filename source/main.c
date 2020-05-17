/*	Author: agonz250 
 *  Partner(s) Name: 
 *	Lab Section: 028
 *	Assignment: Lab #9  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//////////////////////////////////
//timer stuff 
volatile unsigned char TimerFlag = 0; //stuff added 

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	//AVR timer/counter controller register ....
	TCCR1B = 0x0B;

	//AVR output compare register ....
	OCR1A = 125;

	//AVR timer interrupt mask register
	TIMSK1 = 0x02;

	//initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerIRS called every _avr ... milliseconds
	//
	
	//enable global interrupts
	SREG |= 0x80; //0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; //timer off
}

void TimerISR() {
	TimerFlag = 1;
}

//TimerISR() calls this 
ISR(TIMER1_COMPA_vect) {
	//cpu calls when TCNT1 == OCR1
	_avr_timer_cntcurr--;	//counts down to 0

	if (_avr_timer_cntcurr == 0) {
		TimerISR();	//calls ISR that user uses
		_avr_timer_cntcurr = _avr_timer_M;

	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}



/////////////////////////////////////////////////////
//stuff for PWM
void set_PWM(double frequency) {
	static double current_frequency;

	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0XFFFF; }

		else if (frequency > 31250) { OCR3A = 0x0000;}

		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0;
		current_frequency = frequency; 

	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);

	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

//~~~~~~~~~~~~~~~~~~~~~
//STATE STUFF 


enum States {Start, Wait, Inc, Dec, Toggle} state;

unsigned char tmpA;

double arr[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};

//unsigned char tmpB;
unsigned char i;

unsigned char soundON = 1; //Acts like a bool, determines if sound is on or not 

void Tick() {
	switch (state) {
		case Start:
			//tmpB = 0;
			state = Wait;
			i = 0;
			soundON = 1;
			PWM_on();
			set_PWM(arr[i]);
			break;

		case Wait: 
			if ( (tmpA & 0xFF) == 0x01) { //PA0, inc
				if (i < 7) { //If not at end of array
					i++;
				}

				if (soundON) { //If the sound is on, set the tone
					set_PWM(arr[i]);
				}	
				state = Inc;
			}	
			else if ( (tmpA & 0xFF) == 0x02) { //PA1, dec
				if (i > 0) { //can't be less than 0
					i--;
				}

				if (soundON) {
					set_PWM(arr[i]);
				}	

				state = Dec;
			}
			else if ( (tmpA & 0xFF) == 0x04) { //Toggle, PA2
				set_PWM(0); //E

				if (soundON == 1) { //If the sound was on, turn off
					soundON = 0;
				}
				else { //if off, turn it back on
					soundON = 1;
				}
				//soundON = 0;
				state = Toggle;
			}
			else if ( (tmpA & 0xFF) == 0x00) { //If no butt pressed
				//set_PWM(0); //C

				if (soundON) {
					set_PWM(arr[i]);
				}	
				state = Wait;
			}
			else {

				if (soundON) {
					set_PWM(arr[i]);
				}	
				state = Wait;
			}
			//else { //Else multiple buttons are pressed 
			//	state = manyButtHold;
			//
			break;
			
		case Inc:
			if ( (tmpA & 0xFF) == 0x01) { //PA0
				state = Inc;
			}/*	
			else if ( (tmpA & 0xFF) == 0x02) { //PA1
				state = oneButtHold;
			}
			else if ( (tmpA & 0xFF) == 0x04) { //PA2
				state = oneButtHold;
			}*/
			else if ( (tmpA & 0xFF) == 0x00) { //If no butt pressed
				//set_PWM(0); //C
				state = Wait;
			}/*
			else { //Else multiple buttons are pressed 
				state = manyButtHold;
			}*/
			break;


		case Dec:
			if ( (tmpA & 0xFF) == 0x00) { //If buttons released
				//set_PWM(0); //C
				state = Wait;
			}
			else { //Else, buttons must be on, so stay here
				state = Dec;
			}
			break;

		case Toggle:
			if ( (tmpA & 0xFF) == 0x04) {
				state = Toggle;
			}
			else {
				state = Wait;
			}
			break;

		default:
			state = Wait;
			//set_PWM(0);
			break;
	}

	switch(state) { //state actions
		case Wait:
			//set_PWM(0);
			break;


		case Inc:
			break;

		case Dec:
			//set_PWM(0);
			break;

		case Toggle:
			break;


		default: 
			break;
	}
}


//=====================


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; //input

	DDRB = 0xFF; PORTB = 0x00; //output

	tmpA = 0x00;
	//tmpB = 0x00;
	
	
	//Timer stuff
	TimerSet(200);
	TimerOn();

    /* Insert your solution below */
    while (1) {
	    tmpA = ~PINA;

	    Tick();
	    
	    while(!TimerFlag) {}
	    TimerFlag = 0;

    }
    return 1;
}
