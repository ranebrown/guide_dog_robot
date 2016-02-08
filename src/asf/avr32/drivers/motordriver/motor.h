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

void initMotor(uint32_t resetPin);

char getMotorFirmwareVersion(void);

int getMotorErrors(void);

int getMotorConfigurationParameter(int parameter);

int	setMotorConfigurationParameter(int parameter, int value);

void setM0Speed(int speed);

void setM1Speed(int speed);

void setMotorSpeeds(int m0Speed, int m1Speed);

void setM0Brake(int brake);

void setM1Brake(int brake);

void setMotorBrakes(int m0Brake, int m1Brake);

unsigned char getM0Current(void);

unsigned char getM1Current(void);

unsigned int getM0CurrentMilliamps(void);

unsigned int getM1CurrentMilliamps(void);

unsigned char getM0Speed(void);

unsigned char getM1Speed(void);

#endif // MOTOR_H
