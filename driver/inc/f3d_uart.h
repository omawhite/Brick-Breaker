/* f3d_uart.h --- 
 * 
 * Filename: f3d_uart.h
 * Description: 
 * Author: Bryce Himebaugh
 * Maintainer: 
 * Created: Thu Sep 26 07:05:43 2013
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

/* #include <stm32f10x.h> */
/* #include <stm32f10x_gpio.h> */
/* #include <stm32f10x_rcc.h> */
/* #include <stm32f10x_usart.h> */



void f3d_uart_init(void);
int putchar(int);
int getchar(void);
void putstring(char *);

/* f3d_uart.h ends here */
