/*
 * IncFile1.h
 *
 * Created: 2/24/2016 3:37:25 PM
 *  Author: maor4190
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#include <i2cfun.h>
#include <stdint.h>

/**
 * @brief function initializes communication with a wii nunchuck remote
 *		nunchuck uses I2C comms address 0x52, in order it communicate it must receive the sequence
 *		0x40, 0x00
 */
void initNunchuck();

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
void getNunchuckData(char buff[6]);

/** 
 * @brief decodes the z and c buttons of nunchuck
 *		0 not pressed, 1 pressed
 *		6th byte of nunchuck data packet contains button info
 *		bit 0 == z button, bit 1 == c button
 * @param[in] buff the buffer containing 6 bytes of nunchuck data
 * @param[out] z pointer to a character -> z button status
 * @param[out] c pointer to a character -> c button status
 */
void getNunchuckButtons(char buff[6], uint8_t *z, uint8_t *c);

/**
 * @brief extracts the x-axis and y-axis data of the nunchuck joystick
 *		byte 0 = x-axis, byte 1 = y-axis
 * @param[in] buff the nunchuck buffer containing 6 bytes of data
 * @param[out] x_axis pointer to location for x-axis data to be stored
 * @param[out] y_axis pointer to location for y-axis data to be stored
 */
void getNunchuckJoy(char buff[6], uint8_t *x_axis, uint8_t *y_axis);

#endif /* INCFILE1_H_ */