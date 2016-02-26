/*
 * CFile1.c
 *
 * Created: 2/24/2016 3:37:02 PM
 *  Author: maor4190
 */ 

#include "nunchuck.h"

void getNunchuckData(char *buff, char *z, char *c)
{
	char zero = 0x00;
	char zeroP = &zero;
	int i=0;
	char temp = 0;
	int d =0;
	i2c_write(0x52,zeroP,1);
	for(d=0;d<3000;d++);
	// get 6 bytes from nunchuck
	i2c_read(0x52,6,buff);
	
	// send nunchuck a zero -> some sort of reset
	//i2c_write(0x52,zeroP,1);
	
	// decode data
	char val[6];
	for(i=0; i<6; i++)
		val[i] = ((*(buff+i)) ^ 0x17) + 0x17;
		
	// buff[0] x-axis of joystick
	// buff[1] y-axis of joystick
	// buff[2] x-axis of accelerometer
	// buff[3] y-axis of accelerometer
	// buff[4] z-axis of accelerometer
	// buff[5] c and z buttons
	
	// extract button info
	// 1 == pressed, 0 == not pressed
	temp = (val[5] >>0) & 1;
	if(temp == 1)
		*z = 0;
	else
		*z = 1;
	temp = (val[5] >>1) & 1;
	if(temp == 1)
		*c = 0;
	else 
		*c = 1;
}