#include "motor.h"
#include "gpio.h"
#include "usart.h"

uint8_t cmd[5]; // serial command buffer

/* 
 * initilize the motor controller
 */
void initMotor(uint32_t resetPin) {
	// uart options set inside init.c
	uint32_t *USART2_BASE_ADDR = (uint32_t*)0xFFFF3800;

	// reset qik
	// pulling pin low resests qik (normally pulled high)
	gpio_enable_gpio_pin(resetPin);
	gpio_configure_pin(resetPin, GPIO_DIR_OUTPUT);
	gpio_set_pin_low(resetPin);
	gpio_configure_pin(resetPin, GPIO_DIR_INPUT);

	// set qik to auto detect baud rate
	int success = usart_putchar(USART2_BASE_ADDR, 0xAA);
	if(success != USART_SUCCESS)
		// TODO error handling
		
}

/*
 * request the firmware version of motor controller 
 * returns firmware version for success, -1 otherwise
 */
char getMotorFirmwareVersion(void) {
	// variables
	int version = 0;
	int *vptr = &version;
	int success = -1
	
	// send request for firmware version
	success = usart_putchar(USART2_BASE_ADDR, QIK_GET_FIRMWARE_VERSION);
	if(success != USART_SUCCESS)
		return -1;
	else {
		success = -1;
		// wait for data from buffer
		while(USART_RX_EMPTY) {
			success = usart_read_char(USART2_BASE_ADDR, vptr);
		}
		// if data was read return that data
		if(success == USART_SUCCESS)
			return (char)version;
		else
			return -1;
	}
}

/*
 * Request any errors from motor controller
 * bit 0: motor 0 fault
 * bit 1: motor 1 fault
 * bit 2 motor 0 over current
 * bit 3 motor 2 over current
 * bit 4 serial hardware fault
 * bit 5 CRC error
 * bit 6 format error
 * bit 7 timeout
 * returns 8 bit error code for success, -1 for failure
 *
 */
int gettMotorErrors(void) {
	// variables
	int err = 0;
	int *eptr = &err;
	int success = -1
	
	// send request for errors
	success = usart_putchar(USART2_BASE_ADDR, QIK_GET_ERROR_BYTE);
	if(success != USART_SUCCESS)
		return -1; // error
	else {
		success = -1;
		// wait for data from buffer
		while(USART_RX_EMPTY) {
			success = usart_read_char(USART2_BASE_ADDR, eptr);
		}
		// return errors
		if(success == USART_SUCCESS)
			return err;
		else
			return -1; // error
	}
}


uint8_t gettMotorConfigurationParameter(uint8_t parameter) {
	uint8_t res;

	return res;
}

uint8_t	settMotorConfigurationParameter(uint8_t parameter, uint8_t value) {
	uint8_t res;

	return res;
}

void setM0Speed(int speed) {

}

void setM1Speed(int speed) {

}

void settMotorSpeeds(int m0Speed, int m1Speed) {

}

void setM0Brake(unsigned char brake) {

}

void setM1Brake(unsigned char brake) {

}

void settMotorBrakes(unsigned char m0Brake, unsigned char m1Brake) {

}

unsigned char getM0Current(void) {
	unsigned char current;

	return current;
}

unsigned char getM1Current(void) {
	unsigned char current;

	return current;
}

unsigned int getM0CurrentMilliamps(void) {
	unsigned int current;

	return current;

}

unsigned int getM1CurrentMilliamps(void) {
	unsigned int current;

	return current;

}

unsigned char getM0Speed(void) {
	unsigned char speed;

	return speed;
}

unsigned char getM1Speed(void) {
	unsigned char speed;

	return speed;

}

