

#ifndef _SPIFUN_
#define _SPIFUN_
#include <stdint.h>
#ifndef ASF_H
#include <asf.h>
#define ASF_H
#endif
#include "asf/avr32/drivers/spi/spi.h"

/************************************************************************/
/*	 \brief Unlock the flash chip                                                                     
*/
/************************************************************************/
extern int spi_unlock_flash();


/************************************************************************/
/*	\brief Erase a 64k sector        

	\param addr32			start adress to erase from (multiple of 64k)                                                             
*/
/************************************************************************/
extern int spi_64k_erase(int addr32);


/************************************************************************/
/*	\brief Erase the whole flash chip                                                                     
*/
/************************************************************************/
extern int spi_chip_erase();


/************************************************************************/
/*	\brief Write byte to the flash chip
    
	\param addr32			address to write to
	\param data				data to write                                                                
 */
/************************************************************************/
extern int spi_write_flash(int addr32, int data);


/************************************************************************/
/*	\brief Read byte from flash  

	\param addr32			address to read from                                                                   
*/
/************************************************************************/
extern unsigned int spi_read_flash(int addr32);


/************************************************************************/
/*	\brief Return the value of the flash chips status register (see datasheet)                                                                      
*/
/************************************************************************/
extern int spi_read_status();

#endif