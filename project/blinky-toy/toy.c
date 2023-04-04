#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"
#include "led.h"

#define SW1 BIT0
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3
#define SWITCHES (SW1 | SW2 | SW3 | SW4)

int button_state = 0;

int laser = 3500;
int second_count = 0;
int blink_count = 0;

int main() {
    configureClocks();

    P1DIR |= LEDS;
    P1OUT &= ~LED_GREEN;
    P1OUT &= ~LED_RED;
    
    P2REN |= SWITCHES;
    P2IE |= SWITCHES;
    P2OUT |= SWITCHES;
    P2DIR &= ~SWITCHES;
    
    buzzer_init();
    // buzzer_set_period(0);	/* start buzzing!!! 2MHz/1000 = 2kHz*/

    
    or_sr(0x18);          // CPU off, GIE on
}

void
switch_interrupt_handler()
{
  char p2val = P2IN;
  
  P2IES |= (p2val & SWITCHES);
  P2IES &= (p2val | ~SWITCHES);

  
  /* Button 1 */
  if (!(p2val & SW1)) {
    enableWDTInterrupts();
  }
  
  /* Button 2 */
  else if (!(p2val & SW2)) {
    buzzer_set_period(3000);
  }
  
  /* Button 3 */
  else if (!(p2val & SW3)) {
    buzzer_set_period(1500);
  }

  /* Button 4 */
  else if (!(p2val & SW4) & button_state == 0) {
    buzzer_set_period(0);
    button_state = 1;
  }

  else if (!(p2val & SW4) & button_state == 1) {
    buzzer_set_period(5000);
    button_state = 0;
  }
  
  /* Button Release */
  else {
    if (button_state == 0) {
      buzzer_set_period(0);
      P1OUT &= ~LEDS;
      configureClocks();
    }
    
    else if (button_state == 1) {
      buzzer_set_period(5000);
      P1OUT |= LEDS;
      configureClocks();
    }
    
    else
      {
	buzzer_set_period(0);
	configureClocks();
	P1OUT &= ~LEDS;
      }
  }
}

void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {
    P2IFG &= ~SWITCHES;
    switch_interrupt_handler();
  }
}

void 
__interrupt_vec(WDT_VECTOR) WDT(){
    blink_count ++;
    if (blink_count >= 8)
      {
	blink_count = 0;
	P1OUT |= LED_RED;
      }
    
    else
      P1OUT &= ~LED_RED;
	
    second_count ++;
    laser -= 50;
    buzzer_set_period(laser);
    if (second_count >= 50) {
      second_count = 0;
      laser = 3500;
    }
}
