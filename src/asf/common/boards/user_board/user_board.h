/*
*	user_board.h
*	description: contains definitions and function prototypes for custom PCB
*/

#ifndef USER_BOARD_H
#define USER_BOARD_H

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
void board_init();

/*
* description: initializes gpio pins
* args: none
* return: none
*/
void initClock();

/*
* description: outputs clock signal to gpio pin 6
* args: none
* return: none
*/
void gpioClock();

#endif // USER_BOARD_H
