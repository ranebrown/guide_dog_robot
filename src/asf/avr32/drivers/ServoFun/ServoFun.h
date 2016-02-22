#ifndef _SERVOFUN_
#define _SERVOFUN_
#include <stdint.h>

#define MIN_PULSE_WIDTH       544		 // the shortest pulse sent to a servo
#define MAX_PULSE_WIDTH      2400		// the longest pulse sent to a servo
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define SERVO_MIN() MIN_PULSE_WIDTH 	//- this->min * 4)   // minimum value in uS for this servo
#define SERVO_MAX() MAX_PULSE_WIDTH 	//- this->max * 4)   // maximum value in uS for this servo
#define TRIM_DURATION  5                                   // compensation ticks to trim adjust for digitalWrite delays
//#define usToTicks(int us)    ((39.5 * _us) / 16)                 // converts microseconds to tick


long map(long x, long in_min, long in_max, long out_min, long out_max); //maps angle input to servo pulse width

void write(int angle);				// if value is < 200 its treated as an angle, otherwise as pulse width in microseconds
void writeMicroseconds(int angle); // Write pulse width in microseconds

/*
 * moves the servo to its predefined home location
 * home location - right side at 45 deg angle
 */
void sweepHome();

/*
 * sweeps lidar from home position to left 
 * sweep moves through 90 deg
 * stops at left side
 */
void sweepLeft();

/*
 * sweeps lidar from far left position to home position
 * sweep moves through 90 deg
 * stops at home position
 */
void sweepRight();

/*
 * sweeps from home position to left and then back home
 * sweep moves through 180 deg
 */
void sweepFull();

/*
 * sweeps left x degrees
 * max left movement is 90 deg from home location
 */
void sweepLeftXDeg(float x);

/*
 * sweeps right x degrees
 * max right movement is home position (45 deg)
 */
void sweepRightXDeg(float x);

#endif