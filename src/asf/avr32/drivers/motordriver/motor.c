#include "motor.h"

uint8_t cmd[5]; // serial command buffer

void initMotor(long baud, unsigned char receivePin, unsigned char transmitPin, unsigned char resetPin) {

}

char getMotorFirmwareVersion(void) {
	char version;

	return version;
}

uint8_t gettMotorErrors(void) {
	uint8_t error;

	return error;
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

