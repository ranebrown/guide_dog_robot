/*
*	user_board.h
*	description: contains definitions and function prototypes for custom PCB
*/

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include "eic.h"
#include "uc3l0256.h"

// definitions
#define WRITE_MR(val) ((*(volatile uint32_t *)0xFFFF4004) = (val))
#define WRITE_CR(val) ((*(volatile uint32_t *)0xFFFF4000) = (val))
#define WRITE_CS0(val) ((*(volatile uint32_t *)0xFFFF4030) = (val))
#define WRITE_CS3(val) ((*(volatile uint32_t *)0xFFFF403C) = (val))
#define WRITE_TDR(val) ((*(volatile uint32_t *)0xFFFF400C) = (val))
#define AVR32_GC_DIV_CLOCK		0x00000001

/*
* description: initializes PCB and peripherals
* args: none
* return: none
*/
void board_init(void);

/*
* description: initializes gpio pins
* args: none
* return: none
*/
void initClock(void);

/*
* description: outputs clock signal to gpio pin 6
* args: none
* return: none
*/
void gpioClock(void);

/*
 * initializes interrupt
 * param eic_options = structure containing interrupt settings
 * return 0 success, 1 else
 */
void initInterrupt(void);

// interrupt handler
static void eic_int_handler1(void);

#endif // USER_BOARD_H
