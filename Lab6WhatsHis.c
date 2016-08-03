// built-in connection: PF0 connected to negative logic momentary switch, SW2
// built-in connection: PF1 connected to red LED
// built-in connection: PF2 connected to blue LED
// built-in connection: PF3 connected to green LED
// built-in connection: PF4 connected to negative logic momentary switch, SW1

// Edited by Aleksey

#include "TExaS.h"
#include <TM4C123GH6PM.h> // Register Definitions 

#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control
#define PF4                     (*((volatile unsigned long *)0x40025040))
#define PF3                     (*((volatile unsigned long *)0x40025020))
#define PF2                     (*((volatile unsigned long *)0x40025010))
#define PF1                     (*((volatile unsigned long *)0x40025008))
#define PF0                     (*((volatile unsigned long *)0x40025004))
	
#define DELAY 100 // define delay, ms

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

//		Function Prototypes
void PortF_Initialization (void);
void Delay100ms(unsigned long time);

// Global variables
unsigned long SW1;

int main(void){ 
	
  TExaS_Init(SW_PIN_PF4, LED_PIN_PF2);  // activate grader and set system clock to 80 MHz
  PortF_Initialization ();							// initialization goes here
  EnableInterrupts();         					// enable interrupts for the grader
	
	GPIO_PORTF_DATA_R |= 0x04; 				// BLUE LED ON
  
	while(1){
		
		Delay100ms(DELAY/100); //  Delay about 100 ms
		
		SW1 = PF4;  // Read the switch SW1 on PF4
								// or SW1 = GPIO_PORTF_DATA_R &0x10;
		
		if (!SW1){
			GPIO_PORTF_DATA_R ^= 0x04; // toggle PF2, LED on-off
		}
		else
		{
			GPIO_PORTF_DATA_R |= 0x04; // set PF2, LED on
		}
			
  }
	
}

void PortF_Initialization (void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |=0x20; 							// Enable clock for PortF, RCGC2 "bit5"
	delay = SYSCTL_RCGC2_GPIOF; 				// Make some an abstract operation for delay
	
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   	// Unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           	// Allow changes to PF4-0
																			// Only PF0 needs to be unlocked, other bits can't be locked
	
	GPIO_PORTF_AMSEL_R = 0x00; 					// Disable analog functions
	GPIO_PORTF_PCTL_R = 0x00000000; 		// Disable alternative functions. Just digital use
	GPIO_PORTF_DIR_R = 0x0E;						// PF1-PF3 outputs, PF0 and PF4 inputs
	GPIO_PORTF_AFSEL_R = 0x00;					// Disable alt functions on PF7-PF0
	GPIO_PORTF_PUR_R = 0x11;						// Set pull-up on PF0, PF4
	GPIO_PORTF_DEN_R = 0x1F;						// Enable digital I/O on PF0-PF4
	}

void Delay100ms(unsigned long time){
  unsigned long i;
  while(time > 0){
    i = 1333333;  // this number means 100ms
    while(i > 0){
      i = i - 1;
    }
    time = time - 1; // decrements every 100 ms
  }
}
