#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// Commands
#define QIK_GET_FIRMWARE_VERSION         0x81
#define QIK_GET_ERROR_BYTE               0x82
#define QIK_GET_CONFIGURATION_PARAMETER  0x83
#define QIK_SET_CONFIGURATION_PARAMETER  0x84

#define QIK_MOTOR_M0_FORWARD             0x88
#define QIK_MOTOR_M0_FORWARD_8_BIT       0x89
#define QIK_MOTOR_M0_REVERSE             0x8A
#define QIK_MOTOR_M0_REVERSE_8_BIT       0x8B
#define QIK_MOTOR_M1_FORWARD             0x8C
#define QIK_MOTOR_M1_FORWARD_8_BIT       0x8D
#define QIK_MOTOR_M1_REVERSE             0x8E
#define QIK_MOTOR_M1_REVERSE_8_BIT       0x8F
#define QIK_MOTOR_M0_BRAKE	       		 0x86
#define QIK_MOTOR_M1_BRAKE       		 0x87

#define QIK_GET_MOTOR_M0_CURRENT 		 0x90
#define QIK_GET_MOTOR_M1_CURRENT 		 0x91
#define QIK_GET_MOTOR_M0_SPEED   		 0x92
#define QIK_GET_MOTOR_M1_SPEED   		 0x93

// Configuration parameters
#define QIK_CONFIG_DEVICE_ID                        0
#define QIK_CONFIG_PWM_PARAMETER                    1
#define QIK_CONFIG_SHUT_DOWN_MOTORS_ON_ERROR        2
#define QIK_CONFIG_SERIAL_TIMEOUT                   3
#define QIK_CONFIG_MOTOR_M0_ACCELERATION            4
#define QIK_CONFIG_MOTOR_M1_ACCELERATION            5
#define QIK_CONFIG_MOTOR_M0_BRAKE_DURATION          6
#define QIK_CONFIG_MOTOR_M1_BRAKE_DURATION          7
#define QIK_CONFIG_MOTOR_M0_CURRENT_LIMIT_DIV_2     8
#define QIK_CONFIG_MOTOR_M1_CURRENT_LIMIT_DIV_2     9
#define QIK_CONFIG_MOTOR_M0_CURRENT_LIMIT_RESPONSE 10
#define QIK_CONFIG_MOTOR_M1_CURRENT_LIMIT_RESPONSE 11

/* 
 * initilize the motor controller
 */
void initMotor(uint32_t resetPin);

/*
 * request the firmware version of motor controller 
 * returns firmware version for success, -1 otherwise
 */
char getMotorFirmwareVersion(void);

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
int getMotorErrors(void);

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
int getMotorConfigurationParameter(int parameter);

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
int	setMotorConfigurationParameter(int parameter, int value);

/* sets motor m0 speed
 * speed values -127 to 127 (negative values are for reverse)
 */
void setM0Speed(int speed);

/* sets motor m1 speed
 * speed values -127 to 127 (negative values are for reverse)
 */
void setM1Speed(int speed);

/* sets motor m0 and m1 speed
 * speed values -127 to 127 (negative values are for reverse)
 */
void setMotorSpeeds(int m0Speed, int m1Speed);

/* sets brake for motor m0
 * a value of 0 is no brake (free spin)
 * a value of 127 is full brake
 */
void setM0Brake(int brake);

/* sets brake for motor m1
 * a value of 0 is no brake (free spin)
 * a value of 127 is full brake
 */
void setM1Brake(int brake);

/* sets brake for motor m0 and m1
 * a value of 0 is no brake (free spin)
 * a value of 127 is full brake
 */
void setMotorBrakes(int m0Brake, int m1Brake);

/*
 * returns an 8 bit value of the average motor m0 current of the last 5ms
 * if uart error returns -1
 * NOTE: actual current can vary by as much as 20%
 */
int getM0Current(void);

/*
 * returns an 8 bit value of the average motor m1 current of the last 5ms
 * NOTE: actual current can vary by as much as 20%
 */
int getM1Current(void);

/*
 * returns an 8 bit value of current in mA for motor m0
 * returns -1 for uart error
 */
int getM0CurrentMilliamps(void);

/*
 * returns an 8 bit value of current in mA for motor m1
 * returns -1 for uart error
 */
int getM1CurrentMilliamps(void);

/* returns the speed the qik is attempting to set motor m0 to
 * returns -1 for error
 * this is not the actual motor speed -> use encoder for the actual speed
 */
int getM0Speed(void);

/* returns the speed the qik is attempting to set motor m1 to
 * returns -1 for error
 * this is not the actual motor speed -> use encoder for the actual speed
 */
int getM1Speed(void);

#endif // MOTOR_H
