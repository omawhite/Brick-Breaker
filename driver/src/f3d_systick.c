/* f3d_systick.c ---
 *
 * Filename: f3d_systick.c
 * Description:
 * Author: Bryce Himebaugh
 * Maintainer:
 * Created: Thu Nov 14 07:57:37 2013
 * Last-Updated:
 *           By:
 *     Update #: 0
 * Keywords:
 * Compatibility:
 *
 */

/* Commentary:
 *
 *
 *
 */

/* Change log:
 *
 *
 */

/* Copyright (c) 2004-2007 The Trustees of Indiana University and
 * Indiana University Research and Technology Corporation.
 *
 * All rights reserved.
 *
 * Additional copyrights may follow
 */

/* Code: */

#include <f3d_systick.h>
#include <f3d_led.h>
#include <f3d_user_btn.h>
#include <math.h>
#include <f3d_uart.h>
#include <queue.h>

volatile int systick_flag = 0;
int counter = 0;
int activeLed = 8;
int prevLed = 15;
struct queue txbuf;

void f3d_systick_init(void) {

  SysTick_Config(SystemCoreClock/SYSTICK_INT_SEC);
}

void SysTick_Handler(void){

  putchar(getchar()); 
  if(!queue_empty(&txbuf)){
    flush_uart();
  }

  /*
  //for the led slowdown experiment
  if(user_btn_read() == 1){
    SysTick_Config(SystemCoreClock/(SYSTICK_INT_SEC / 10));
  }else{
    SysTick_Config(SystemCoreClock/SYSTICK_INT_SEC);
  }
  f3d_led_on(activeLed);
  f3d_led_off(prevLed);
  counter++;
  prevLed = activeLed;
  activeLed = (abs(counter)%8) + 8;
  */
}


/* f3d_systick.c ends here */
