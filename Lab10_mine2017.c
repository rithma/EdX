// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5         
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#include <stdint.h>

#define SENSORS        (*((volatile uint32_t *)0x4002401C))         //Port E bit 2,1,0
#define TRAFLIGHT      (*((volatile uint32_t *)0x400050FC))         //	PB5 thru PB0
#define PEDLIGHT       (*((volatile uint32_t *)0x40025028))         //CHECK THIS VALUE FOR PF3, PF1 *****UNCHECKED******

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void);                                       // Disable interrupts
void EnableInterrupts(void);                                        // Enable interrupts
void ports_Init(void);

typedef struct State {
	uint32_t trafOut;                                                 //6 bit PB5 thru PB0
	uint32_t pedOut;                                                  // PF3, PF1
	uint32_t delay;
  uint32_t next[9	];	
}              STyp;

#define goW      0
#define waitW    1
#define goS      2
#define waitS    3
#define walk     4
#define flash1   5
#define flash2   6
#define flash3   7
#define flash4   8

/*
STyp FSM[9]={
	{0x0C, 0x02, 500, {0,0,1,1,1,1,1,1}}, //0
	{0x14, 0x02, 50,  {2,2,2,2,4,0,2,2}}, //1
	{0x21, 0x02, 500, {2,3,2,3,2,3,2,3}},//2
	{0x22, 0x02, 50,  {0,0,0,0,4,0,0,4}},//3
	{0x24, 0x08, 500, {4,5,5,5,4,5,5,5}},//4
	{0x24, 0x02, 50,  {6,6,6,6,6,6,6,6}},//5
	{0x24, 0x00, 50,  {7,7,7,7,7,7,7,7}},//6
	{0x24, 0x02, 50,  {8,8,8,8,8,8,8,8}},//7
	{0x24, 0x00, 50,  {0,0,2,0,4,0,2,0}},//8
};
*/

STyp FSM [9] = {  //  nada | southWt | westWait | both | walkWait | walkSouth | walkWest | all 
	{ 0x21, 0x2, 500, { goW,	  waitW,	 goW,	     waitW,	 waitW,	    waitW,	    waitW,	   waitW    }},    //goW
	{ 0x22, 0x2, 50,  { goS,    goS,     goS,      goS,    walk,      walk,       walk,      goS      }},    //waitW
	{ 0x0C, 0x2, 100, { goS,    goS,     waitS,    waitS,  waitS,     waitS,      waitS,     waitS    }},    //goS      2
	{ 0x14, 0x2, 50,  { walk,   walk,    goW,      goW,    walk,       walk,       walk,      walk     }},    //waitS     
  { 0x24, 0x8, 100, { walk,   flash1,  flash1,   flash1, walk,      flash1,     flash1,    flash1    }},    //walk  4
  { 0x24, 0x2, 50,  { flash2, flash2,  flash2,   flash2, flash2,    flash2,     flash2,    flash2	  }},    //flash1   5
	{ 0x24, 0x0, 50,  { flash3, flash3,  flash3,   flash3, flash3,    flash3,     flash3,    flash3   }},    //flash2
	{ 0x24, 0x2, 50,  { flash4, flash4,  flash4,   flash4, flash4,    flash4,     flash4,    flash4   }},    //flash3
	{ 0x24, 0x0, 50,  { walk,   goS,     goW,      goW,    walk,      goS,        goW,       goW      }}     //flash4
	
};


uint32_t S;         //index to the next state
uint32_t Input;


// ***** 3. Subroutines Section *****

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	ports_Init();
	SysTick_Init();
	
 
  
  EnableInterrupts();
	
	S = goW;
	
  while(1){
		TRAFLIGHT = FSM[S].trafOut;
		PEDLIGHT = FSM[S].pedOut;
		SysTick_Wait10ms(FSM[S].delay);
		Input = SENSORS;
		S = FSM[S].next[Input];
     
  }
}


void ports_Init(void){   //Initialize ports B, E, F  
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x32; // activate clock for Port B,E,F
  delay = SYSCTL_RCGC2_R; // allow time for clock to start
	
	// Port B
  GPIO_PORTB_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5-0
	GPIO_PORTB_PCTL_R = 0x00000000;   // clear PCTL
  GPIO_PORTB_AMSEL_R &= ~0x3F;      // disable analog on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F;      // disable alt funct on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;         // enable digital I/O on PB5-0
	GPIO_PORTB_DIR_R |= 0x3F;         // PB5-0 outputs
	
	// Port E
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTE_CR_R = 0x07;           // allow changes to PE2-0
	GPIO_PORTE_PCTL_R = 0x00000000;   // clear PCTL
  GPIO_PORTE_AMSEL_R &= ~0x07;      // disable analog on PE2-0
  GPIO_PORTE_AFSEL_R &= ~0x07;      // disable alt funct on PE2-0
  GPIO_PORTE_PUR_R &= ~0x07;        // disableb pull-up on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;         // enable digital I/O on PE2-0
	GPIO_PORTE_DIR_R &= ~0x07;        // PE2-0 inputs

	// Port F
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock port
  GPIO_PORTF_CR_R = 0x0A;           // allow changes to PF1 & PF3
	GPIO_PORTF_PCTL_R = 0x00000000;   // clear PCTL
  GPIO_PORTF_AMSEL_R &= ~0x0A;      // disable analog on PF1 & PF3
  GPIO_PORTF_AFSEL_R &= ~0x0A;      // disable alt funct on PF1 & PF3
  GPIO_PORTF_DEN_R |= 0x0A;         // enable digital I/O on PF1 & PF3
	GPIO_PORTF_DIR_R |= 0x0A;         // PF1 & PF3 outputs
}



