#include "ServoFun.h"
#include "pwma.h"
#include "genclk.h"
long map(long x, long in_min, long in_max, long out_min, long out_max){
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void servo_write(int angle){
	// treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
	if (angle < MIN_PULSE_WIDTH)
	{
		if (angle < 0)
		angle = 0;
		else if (angle > 180)
		angle = 180;

		angle = map(angle, 0, 180, SERVO_MIN(), SERVO_MAX());
	}
	writeMicroseconds(angle);
}	

void servo_writeMicroseconds(int angle){
	  // calculate and store the values for the given channel
	  
		  if (angle < SERVO_MIN())          // ensure pulse width is valid
		  angle = SERVO_MIN();
		  else if (angle > SERVO_MAX())
		  angle = SERVO_MAX();

		  angle = angle - TRIM_DURATION;
		  angle = ((19.75 * angle) / 16);  // convert to ticks after compensating for interrupt overhead
		  pwma_set_channels_value(&AVR32_PWMA,(1<<AVR32_PWMA_4_PIN),angle/2400);
		  //servos[channel].ticks = angle; // figure out what this does 
	
}