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


enum States {Start, Wait, oneButtHold, manyButtHold} state;

unsigned char tmpA;

//unsigned char tmpB;

void Tick() {
	switch (state) {
		case Start:
			//tmpB = 0;
			state = Wait;
			PWM_on();
			break;

		case Wait: 
			if ( (tmpA & 0xFF) == 0x01) { //PA0
				set_PWM(261.63); //C
				state = oneButtHold;
			}	
			else if ( (tmpA & 0xFF) == 0x02) { //PA1
				set_PWM(293.66); //D
				state = oneButtHold;
			}
			else if ( (tmpA & 0xFF) == 0x04) { //PA2
				set_PWM(329.63); //E
				state = oneButtHold;
			}
			else if ( (tmpA & 0xFF) == 0x00) { //If no butt pressed
				set_PWM(0); //C
				state = Wait;
			}
			else { //Else multiple buttons are pressed 
				state = manyButtHold;
			}
			break;
			
		case oneButtHold:
			if ( (tmpA & 0xFF) == 0x01) { //PA0
				state = oneButtHold;
			}	
			else if ( (tmpA & 0xFF) == 0x02) { //PA1
				state = oneButtHold;
			}
			else if ( (tmpA & 0xFF) == 0x04) { //PA2
				state = oneButtHold;
			}
			else if ( (tmpA & 0xFF) == 0x00) { //If no butt pressed
				set_PWM(0); //C
				state = Wait;
			}
			else { //Else multiple buttons are pressed 
				state = manyButtHold;
			}
			break;


		case manyButtHold:
			if ( (tmpA & 0xFF) == 0x00) { //If buttons released
				set_PWM(0); //C
				state = Wait;
			}
			else { //Else, buttons must be on, so stay here
				state = manyButtHold;
			}
			break;

		default:
			state = Wait;
			set_PWM(0);
			break;
	}

	switch(state) { //state actions
		case Wait:
			set_PWM(0);
			break;


		case oneButtHold:
			break;

		case manyButtHold:
			set_PWM(0);
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

    /* Insert your solution below */
    while (1) {
	    tmpA = ~PINA;

	    Tick();

    }
    return 1;
}
