/*
 * CFile1.c
 *
 * Created: 2/24/2016 3:37:02 PM
 *  Author: maor4190
 */ 

#include "nunchuck.h"

/**
 * @brief function initializes communication with a wii nunchuck remote
 *		nunchuck uses I2C comms address 0x52, in order it communicate it must receive the sequence
 *		0x40, 0x00
 */
void initNunchuck()
{
	// handshake sequence
	char setup[2] = {0x40,0x00};
	
	// send handshake
	i2c_write(0x52,setup,2);
}

/**
 * @brief function fetches a data packet of 6 bytes from wii nunchuck
 *		sending a 0x00 to the nunchuck makes the remote store a new data packet in its buffer
 *		these 6 bytes can then be read at address 0x52
 *		!!! it is essential to have a sufficient delay between write and read operations !!!
 * @param[out] buff a 6 byte buffer used to store data
 *
 * buff[0] x-axis of joystick
 * buff[1] y-axis of joystick
 * buff[2] x-axis of accelerometer
 * buff[3] y-axis of accelerometer
 * buff[4] z-axis of accelerometer
 * buff[5] bit 0 z button bit 1 c button
 */
void getNunchuckData(char buff[6])
{
	// variables
	char zero = 0x00;
	int i=0;
	char temp = 0;
	
	// send 0x00 to nunchuck -> new data packet stored in nunchuck buffer
	i2c_write(0x52,&zero,1);
	
	// delay
	for(i=0;i<3000;i++);
	
	// read 6 bytes from nunchuck and store in buff
	i2c_read(0x52,6,buff);	
	
	// decode data
	for(i=0; i<6; i++)
		buff[i] = (buff[i] ^ 0x17) + 0x17;
}				

/** 
 * @brief decodes the z and c buttons of nunchuck
 *		0 not pressed, 1 pressed
 *		6th byte of nunchuck data packet contains button info
 *		bit 0 == z button, bit 1 == c button
 * @param[in] buff the buffer containing 6 bytes of nunchuck data
 * @param[out] z pointer to z button status
 * @param[out] c pointer to c button status
 */
void getNunchuckButtons(char buff[6], uint8_t *z, uint8_t *c)
{
	uint8_t button = buff[5];
	*z = !(button & 1);
	*c = !(button & 2);
}

/**
 * @brief extracts the x-axis and y-axis data of the nunchuck joystick
 *		byte 0 = x-axis, byte 1 = y-axis
 * @param[in] buff the nunchuck buffer containing 6 bytes of data
 * @param[out] x_axis pointer to location for x-axis data to be stored
 * @param[out] y_axis pointer to location for y-axis data to be stored
 */
void getNunchuckJoy(char buff[6], uint8_t *x_axis, uint8_t *y_axis)
{
	*x_axis = buff[0];
	*y_axis = buff[1];
}	