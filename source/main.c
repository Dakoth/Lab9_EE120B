/*	Author: agonz250 
 *  Partner(s) Name: 
 *	Lab Section: 028
 *	Assignment: Lab #9  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
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


enum States {Start, Wait, playNote, downTime, endHold} state;

unsigned char tmpA;

unsigned char i; //, j, k; 
unsigned char x;

//Notes
double C4, D, E, F, G, A, B, C5;
/* 
double C4 = 261.63;
double D = 293.66;
double E = 329.63;
double F = 349.23;
double G = 392.00;
double A = 440.00;
double B = 493.88;
double C5 = 523.25;
*/

/*
double notes[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
double noteLength[8] = {1, 1, 1, 1, 1, 1, 1, 1};
double downT[8] = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
const unsigned char lastIndex = 7;
*/

//double notes[16] = {A, E, D, C5,  B, C5, B, A,  B, C5, D, C5,  B, G, B, A };
double notes[16] = {440, 329.63, 293.66, 523.25,  493.88, 523.25, 493.88, 440,  493.88, 523.25, 293.66, 523.25,  493.88, 392, 493.88, 440};  
//double notes[16] = {233.08, 329.63, 293.66, 523.25,  246.94, 523.25, 246.94, 233.08,  246.94, 523.25, 293.66, 523.25,  246.94, 207.65, 246.94, 233.08};  

double noteLength[16] = {1, 1, 1, 1, 1, 1, 1, 1,  1,1, 1, 1,  1, 1, 1, 1};
double downT[16] = {1, 1, 1, 1, 1, 1, 1, 1,  1,1, 1, 1,  1, 1, 1, 1};
const unsigned char lastIndex = 15; //Last index of the array 
 

void Tick() {
	switch (state) {
		case Start:
			//tmpB = 0;
			state = Wait;
			i = 0;
			x = 0;
			PWM_on();
			//set_PWM(arr[i]);
			break;

		case Wait: 
			if ( (tmpA & 0xFF) == 0x01) { // If button pressed 
				state = playNote;
				x = 0;
			}
			else {
				state = Wait;
			}
			break;
			
		case playNote:
			if (x < noteLength[i]) { //if note not finished playng
				state = playNote;
			}
			else if ( x >= noteLength[i]) { //when note finished
				x = 0;
				state = downTime; 
			}
			break;

		case downTime:
			if ( x < downT[i]) { //when down time not over
				state = downTime;
			}
			else if ( x >= downT[i] && (i < lastIndex) ) { //when haven't reached end of array + downtime over
				i++;
				state = playNote;
			}
			//might need to change tmpA condition a bit; reached end of array
			else if (( x >= downT[i]) && (i >= lastIndex) && (tmpA == 0)) {
				i = 0;
				state = Wait;
			}
			//Goes to endHold
			else if (( x >= downT[i]) && (i >= lastIndex) && (tmpA == 1)) {
				i = 0;
				state = endHold;
			}
			break;

		case endHold:
			if ( (tmpA & 0xFF) == 0x01) {
				state = endHold;
			}
			else {
				state = Wait;
				i = 0;
			}
			break;

		default:
			state = Wait;
			//set_PWM(0);
			break;
	}

	switch(state) { //state actions
		case Wait:
			set_PWM(0);
			break;


		case playNote:
			x++;
			set_PWM(notes[i]);
			break;

		case downTime:
			x++;
			set_PWM(0);
			break;

		case endHold:
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
	TimerSet(150); //500 ms, or .5 of a second 
	TimerOn();


 	/*
	C4 = 261.63;
	D = 293.66;
	E = 329.63;
	F = 349.23;
	G = 392.00;
	A = 440.00;
	B = 493.88;
	C5 = 523.25;
	*/


    /* Insert your solution below */
    while (1) {
	    tmpA = ~PINA;

	    Tick();
	    
	    while(!TimerFlag) {}
	    TimerFlag = 0;

    }
    return 1;
}
