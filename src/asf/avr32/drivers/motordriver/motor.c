#include "motor.h"
#include "gpio.h"
#include "usart.h"

uint32_t *USART2_BASE_ADDRESS = (uint32_t*)0xFFFF3800;
/* 
 * initilize the motor controller
 */
void initMotor(uint32_t resetPin) {
	// uart options set inside init.c

	// reset qik
	// pulling pin low resests qik (normally pulled high)
	gpio_enable_gpio_pin(resetPin);
	gpio_configure_pin(resetPin, GPIO_DIR_OUTPUT);
	gpio_set_pin_low(resetPin);
	gpio_configure_pin(resetPin, GPIO_DIR_INPUT);

	// set qik to auto detect baud rate
	int success = usart_putchar(USART2_BASE_ADDRESS, 0xAA);
	if(success != USART_SUCCESS){}
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
	int success = -1;
	
	// send request for firmware version
	success = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_FIRMWARE_VERSION);
	if(success != USART_SUCCESS)
		return -1;
	else {
		success = USART_RX_EMPTY;
		// wait for data from buffer
		while(success == USART_RX_EMPTY) {
			success = usart_read_char(USART2_BASE_ADDRESS, vptr);
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
	int success = -1;
	
	// send request for errors
	success = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_ERROR_BYTE);
	if(success != USART_SUCCESS)
		return -1; // error
	else {
		success = USART_RX_EMPTY;
		// wait for data from buffer
		while(success == USART_RX_EMPTY) {
			success = usart_read_char(USART2_BASE_ADDRESS, eptr);
		}
		// return errors
		if(success == USART_SUCCESS)
			return err;
		else
			return -1; // error
	}
}

/* 
 * qik has 12 config parameters
 * these parameters have limited r/w and should only by adjusted/read occasionally
 * see manual for more detailed description and acceptable values for the parameters
 * 0: device id
 * 1: PWM 
 * 2: shutdown motors on error
 * 3: serial timeout
 * 4: motor m0 acceleration
 * 5: motor m1 acceleration
 * 6: motor m0 brake duration
 * 7: motor m1 brake duration
 * 8: motor m0 current limit/2
 * 9: motor m1 current limit/2
 * 10: motor m0 current-limit response
 * 11: motor m1 current-limit response
 *
 * returns the value of requested parameter for success, else returns -1
 */
int gettMotorConfigurationParameter(int parameter) {
	// variables
	int res = -1;
	int *rptr = &res;
	int success = -1;
	
	// send request to get the value of parameter
	success = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_CONFIGURATION_PARAMETER);
	if(success != USART_SUCCESS)
		return -1;
	else
		success = usart_putchar(USART2_BASE_ADDRESS, parameter);
	if(success != USART_SUCCESS)
		return -1;
	else {
		success = USART_RX_EMPTY;
		// read parameter value from motor controller
		while(success == USART_RX_EMPTY) {
			success = usart_read_char(USART2_BASE_ADDRESS, rptr);
		}
		if(success == USART_SUCCESS)
			return res;
		else 
			return -1;
	}			
}
/* sets a motor configuration parameter
 * these parameters have limited r/w and should only by adjusted/read occasionally
 * see manual for more detailed description and acceptable values for the parameters
 * 0: device id
 * 1: PWM
 * 2: shutdown motors on error
 * 3: serial timeout
 * 4: motor m0 acceleration
 * 5: motor m1 acceleration
 * 6: motor m0 brake duration
 * 7: motor m1 brake duration
 * 8: motor m0 current limit/2
 * 9: motor m1 current limit/2
 * 10: motor m0 current-limit response
 * 11: motor m1 current-limit response
 * 
 * return -1 for uart error
 * return 0 success
 * return 1 bad parameter number
 * return 2 bad value for parameter
 */
int	settMotorConfigurationParameter(int parameter, int value) {
	// variables
	int res = -1;
	int *rptr = &res;
	int s1=-1, s2=-1, s3=-1, s4=-1, s5=-1;
	int success;
	
	// send request set a parameter value
	// last two bytes are format bytes meant to prevent errors
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_SET_CONFIGURATION_PARAMETER);
	s2 = usart_putchar(USART2_BASE_ADDRESS, parameter);
	s3 = usart_putchar(USART2_BASE_ADDRESS, value);
	s4 = usart_putchar(USART2_BASE_ADDRESS, 0x55);
	s5 = usart_putchar(USART2_BASE_ADDRESS, 0x2A);
	if(s1 != USART_SUCCESS || s2 != USART_SUCCESS || s3 != USART_SUCCESS || s4 != USART_SUCCESS || s5 != USART_SUCCESS)
		return -1; // error sending uart data
	else {
		success = USART_RX_EMPTY;
		// motor controller sends back success or failure in setting the parameter to requested value
		while(success == USART_RX_EMPTY) {
			success = usart_read_char(USART2_BASE_ADDRESS, rptr);
		}
		if(success == USART_SUCCESS)
			return res;
		else
			return -1;
	}
}

/* sets motor m0 speed
 * speed values -127 to 127 (negative values are for reverse)
 */
void setM0Speed(int speed) {
	int reverse = 0;
	int cmd1 = 0, cmd2 = 0;
	int s1 = -1; 
	int s2 = -1;
	
	// check if the requested speed is for reverse
	if(speed < 0) {
		speed = -speed;
		reverse = 1;
	}
	
	// max speed check
	if(speed > 255)
		speed = 255;
	
	// actual speed is an 8 bit value -127 to 127
	if(speed > 127) {
		cmd1 = reverse ? QIK_MOTOR_M0_REVERSE_8_BIT : QIK_MOTOR_M0_FORWARD_8_BIT;
		cmd2 = speed - 128;
	}
	else {
		cmd1 = reverse ? QIK_MOTOR_M0_REVERSE : QIK_MOTOR_M0_FORWARD;
		cmd2 = speed;
	}
	s1 = usart_putchar(USART2_BASE_ADDRESS, cmd1);
	s2 = usart_putchar(USART2_BASE_ADDRESS, cmd2);
	if(s1 != USART_SUCCESS || s2 != USART_SUCCESS){}
		// TODO error handling
}

/* sets motor m1 speed
 * speed values -127 to 127 (negative values are for reverse)
 */
void setM1Speed(int speed) {
	int reverse = 0;
	int cmd1 = 0, cmd2 = 0;
	int s1 = -1; 
	int s2 = -1;
	
	// check if the requested speed is for reverse
	if(speed < 0) {
		speed = -speed;
		reverse = 1;
	}
	
	// max speed check
	if(speed > 255)
	speed = 255;
	
	// actual speed is an 8 bit value -127 to 127
	if(speed > 127) {
		cmd1 = reverse ? QIK_MOTOR_M1_REVERSE_8_BIT : QIK_MOTOR_M1_FORWARD_8_BIT;
		cmd2 = speed - 128;
	}
	else {
		cmd1 = reverse ? QIK_MOTOR_M1_REVERSE : QIK_MOTOR_M1_FORWARD;
		cmd2 = speed;
	}
	s1 = usart_putchar(USART2_BASE_ADDRESS, cmd1);
	s2 = usart_putchar(USART2_BASE_ADDRESS, cmd2);
	if(s1 != USART_SUCCESS || s2 != USART_SUCCESS){}
		// TODO error handling
}

/* sets motor m0 and m1 speed
 * speed values -127 to 127 (negative values are for reverse)
 */
void setMotorSpeeds(int m0Speed, int m1Speed) {
	setM0Speed(m0Speed);
	setM1Speed(m1Speed);
}

/* sets brake for motor m0
 * a value of 0 is no brake (free spin)
 * a value of 127 is full brake
 */
void setM0Brake(int brake) {
	// variables
	int s1 = -1; 
	int s2 = -1;
	
	// set max brake values
	if (brake > 127)
		brake = 127;
	if (brake < 0)
		brake = 0;
	
	// send command
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_MOTOR_M0_BRAKE);
	s2 = usart_putchar(USART2_BASE_ADDRESS, brake);
	if(s1 != USART_SUCCESS || s2 != USART_SUCCESS){}
		// TODO error handling
}

/* sets brake for motor m1
 * a value of 0 is no brake (free spin)
 * a value of 127 is full brake
 */
void setM1Brake(int brake) {
	// variables
	int s1 = -1; 
	int s2 = -1;
	
	// set max brake values
	if (brake > 127)
		brake = 127;
	if (brake < 0)
		brake = 0;
	
	// send command
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_MOTOR_M1_BRAKE);
	s2 = usart_putchar(USART2_BASE_ADDRESS, brake);
	if(s1 != USART_SUCCESS || s2 != USART_SUCCESS){}
		// TODO error handling
}

/* sets brake for motor m0 and m1
 * a value of 0 is no brake (free spin)
 * a value of 127 is full brake
 */
void settMotorBrakes(int m0Brake, int m1Brake) {
	setM0Brake(m0Brake);
	setM1Brake(m1Brake);
}

/*
 * returns an 8 bit value of the average motor m0 current of the last 5ms
 * if uart error returns -1
 * NOTE: actual current can vary by as much as 20%
 */
int getM0Current(void) {
	// variables
	int s1 = -1;
	int current = -1;
	int *cptr = &current;
	
	// send command
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_MOTOR_M0_CURRENT);
	if(s1 != USART_SUCCESS)
		return -1;
	else {
		s1 = USART_RX_EMPTY;
		while(s1 == USART_RX_EMPTY) {
			// get value back from motor controller
			s1 = usart_read_char(USART2_BASE_ADDRESS, cptr);
		}
	}
	if(s1 == USART_SUCCESS)
		return current;
	else
		return -1;
}

/*
 * returns an 8 bit value of the average motor m1 current of the last 5ms
 * NOTE: actual current can vary by as much as 20%
 */
int getM1Current(void) {
	// variables
	int s1 = -1;
	int current = -1;
	int *cptr = &current;
	
	// send command
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_MOTOR_M1_CURRENT);
	if(s1 != USART_SUCCESS)
		return -1;
	else {
		s1 = USART_RX_EMPTY;
		while(s1 == USART_RX_EMPTY) {
			// get value back from motor controller
			s1 = usart_read_char(USART2_BASE_ADDRESS, cptr);
		}
	}
	if(s1 == USART_SUCCESS)
		return current;
	else
		return -1;
}

/*
 * returns an 8 bit value of current in mA for motor m0
 * returns -1 for uart error
 */
int getM0CurrentMilliamps(void) {
	return (getM0Current() * 150);
}

/*
 * returns an 8 bit value of current in mA for motor m1
 * returns -1 for uart error
 */
int getM1CurrentMilliamps(void) {
	return (getM1Current() * 150);
}

/* returns the speed the qik is attempting to set motor m0 to
 * returns -1 for error
 * this is not the actual motor speed -> use encoder for the actual speed
 */
int getM0Speed(void) {
	// variables
	int s1 = -1;
	int speed = -1;
	int *sptr = &speed;
	
	// send command
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_MOTOR_M0_SPEED);
	if(s1 != USART_SUCCESS)
		return -1;
	else {
		s1 = USART_RX_EMPTY;
		while(s1 == USART_RX_EMPTY) {
			// get value back from motor controller
			s1 = usart_read_char(USART2_BASE_ADDRESS, sptr);
		}
	}
	if(s1 == USART_SUCCESS)
		return speed;
	else
		return -1;
}

/* returns the speed the qik is attempting to set motor m1 to
 * returns -1 for error
 * this is not the actual motor speed -> use encoder for the actual speed
 */
int getM1Speed(void) {
	// variables
	int s1 = -1;
	int speed = -1;
	int *sptr = &speed;
	
	// send command
	s1 = usart_putchar(USART2_BASE_ADDRESS, QIK_GET_MOTOR_M1_SPEED);
	if(s1 != USART_SUCCESS)
		return -1;
	else {
		s1 = USART_RX_EMPTY;
		while(s1 == USART_RX_EMPTY) {
			// get value back from motor controller
			s1 = usart_read_char(USART2_BASE_ADDRESS, sptr);
		}
	}
	if(s1 == USART_SUCCESS)
		return speed;
	else
		return -1;
}

int IIR_Motor_Controller(char xin, char *x, char *y){
	//motor constants
	float Kp;
	float Ki;
	float Kd;
	float Ts = 1/95.367;
	float K1 = Kp+Ki*Ts/2+2*Kd/Ts;
	float K2 = Ki*Ts-4*Kd/Ts;
	float K3 = -Kp+Ki*Ts/2+2*Kd/Ts;
	
	//difference equation for IIR filter
	//float y = -y[1] + K1*xin + K2*x[0] + K3*x[1];
	
	//update old values
	//*x[1] = *x[0];
	//*x[0] = xin;
	//*y[1] = y[0];
	//*y[0] = y;
	
	return (int)y;
}

