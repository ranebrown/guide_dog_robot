#ifndef _I2CFUN_
#define _I2CFUN_
#include <stdint.h>

#define MAGNETOMETER_ADDRESS 0x1E


/************************************************************************/
/*	\brief Set i2c bus speed
	
	\param speed			desired i2c bus speed in Hz
	\param pba_hz			pba clock frequency in Hz
 */
/************************************************************************/
int i2c_set_speed(uint32_t speed,uint32_t pba_hz);


/************************************************************************/
/*	\brief Initialize I2C
 */
/************************************************************************/
void i2c_init(void);


/************************************************************************/
/*	\brief Write data to slave

	\param addr			slave addres
	\param data			pointer to data buffer
	\numbytes			number of data bytes
 */
/************************************************************************/
int i2c_write(int addr,char * data,int numbytes);


/************************************************************************/
/*	\brief Read from slave device

	\param addr				slave addres
	\param numbytes			number of bytes to read
 */
/************************************************************************/
int i2c_read(int addr,int numbytes,char *i2c_buffer);

/************************************************************************/
/*	\brief Return Lidar distance measurment
 */
/************************************************************************/
int getLidar(char *i2c_buffer);



void magInit();

int getMagnetometer(int *X, int *Y, int *Z, int *angle,char *i2c_buffer);


#endif