/************************
 *f3d_led.c - contains intializations/functions for the leds
 ************************/

#include <stm32f30x.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <f3d_led.h>

int n;
//intializes the port and pins for the leds on the board
void f3d_led_init(void) {

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
 
}
/*Turns on the appropriate led as specified by the parameter.*/
void f3d_led_on(int led) {
  /*YOUR CODE HERE*/

  switch(led) {

  case 8: GPIOE->BSRR = GPIO_Pin_8;
    break;
  case 9: GPIOE->BSRR = GPIO_Pin_9;
    break;
  case 10: GPIOE->BSRR = GPIO_Pin_10;
    break;
  case 11: GPIOE->BSRR = GPIO_Pin_11;
    break;
  case 12: GPIOE->BSRR = GPIO_Pin_12;
    break;
  case 13: GPIOE->BSRR = GPIO_Pin_13;
    break;
  case 14: GPIOE->BSRR = GPIO_Pin_14;
    break;
  case 15: GPIOE->BSRR = GPIO_Pin_15;
    break;
  default: break;

  }



}

/*Turns off the approiate led as specified by the parameter*/ 
void f3d_led_off(int led) {
  /*YOUR CODE HERE*/

   switch(led) {

  case 8: GPIOE->BRR = GPIO_Pin_8;
    break;
  case 9: GPIOE->BRR = GPIO_Pin_9;
    break;
  case 10: GPIOE->BRR = GPIO_Pin_10;
    break;
  case 11: GPIOE->BRR = GPIO_Pin_11;
    break;
  case 12: GPIOE->BRR = GPIO_Pin_12;
    break;
  case 13: GPIOE->BRR = GPIO_Pin_13;
    break;
  case 14: GPIOE->BRR = GPIO_Pin_14;
    break;
   case 15: GPIOE->BRR = GPIO_Pin_15;
     break;
default: break;
   }

} 

/*Turns on all LEDs*/
void f3d_led_all_on(void) {


  for(n = 8; n < 16; n++) {
    f3d_led_on(n);
  }
 
} 

/*Turns off all LEDs*/
void f3d_led_all_off(void) {
  /*YOUR CODE HERE*/

 for(n = 8; n < 16; n++) {
    f3d_led_off(n);
  }


} 

/* led.c ends here */

