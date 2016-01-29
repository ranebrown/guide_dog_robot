
#include "spifun.h"

int spi_unlock_flash(){
	int i, rx_data;
	//write enable
	*(volatile uint32_t*)0xFFFF400C = 0x06;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK ); //wait for tx buffer to empty
	
	//write status enable
	*(volatile uint32_t*)0xFFFF400C = 0x06;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK ); //wait for tx buffer to empty
	
	//set status reg to 0x00
	*(volatile uint32_t*)0xFFFF400C = (0x01);	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	*(volatile uint32_t*)0xFFFF400C = (0x00);	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	return;
}

int spi_64k_erase(int addr32){
	int i, rx_data;
	//write enable 0x06
	*(volatile uint32_t*)0xFFFF400C = 0x06;																		//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK );			//wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));																//clear rx buffer
	
	char addr2 = (char)((addr32 >> 16) & 0xFF);
	char addr1 = (char)((addr32 >> 8) & 0xFF);
	char addr0 = (char)((addr32 >> 0) & 0xFF);
	char erase_sequence[4] = {(char)0x20,addr2,addr1,addr0};

	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );	//wait for tx buffer to empty

	//send erase sequence
	for (i = 0; i < 4; i++){
		*(volatile uint32_t*)0xFFFF400C = (erase_sequence[i]);													//send data to TX reg
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK );		//wait for rx buffer to be full
		rx_data = *((volatile uint32_t*)(0xFFFF4008));															//clear rx buffer
	}

	return;
}

int spi_chip_erase(){
	int i, rx_data;
	
	//write enable 0x06
	*(volatile uint32_t*)0xFFFF400C = 0x06;																		//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK );			//wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));																//clear rx buffer

	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );	//wait for tx buffer to empty
	
	//chip erase 0x60
	*(volatile uint32_t*)0xFFFF400C = (0x60);																	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK );			//wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));																//clear rx buffer

	return;
}

int spi_write_flash(int addr32, int data){
	int i, rx_data;
	*(volatile uint32_t*)0xFFFF400C = 0x06;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	char addr2 = (char)((addr32 >> 16) & 0xFF);
	char addr1 = (char)((addr32 >> 8) & 0xFF);
	char addr0 = (char)((addr32 >> 0) & 0xFF);
	char write_sequence[5] = {(char)0x02,addr2,addr1,addr0,(char)data};

	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK ); //wait for tx buffer to empty

	for (i = 0; i < 5; i++){
		*(volatile uint32_t*)0xFFFF400C = (write_sequence[i]);	//send data to TX reg
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
		rx_data = *((volatile uint32_t*)(0xFFFF4008));
	}

	return;
}

unsigned int spi_read_flash(int addr32){
	char addr2 = (char)((addr32 >> 16) & 0xFF);
	char addr1 = (char)((addr32 >> 8) & 0xFF);
	char addr0 = (char)((addr32 >> 0) & 0xFF);
	char read_sequence[5] = {(char)0x03,addr2,addr1,addr0,(char)0xFF};
	int i, rx_data;
	for (i = 0; i < 5; i++){
		*(volatile uint32_t*)0xFFFF400C = (read_sequence[i]);	//send data to TX reg
		if (i == 4){
			while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
		}
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
		rx_data = *((volatile uint32_t*)(0xFFFF4008));
	}
	spi_put((uint32_t*)0xFFFF4000, 0x00);
	int dummy = *((volatile uint16_t*)(0xFFFF4008));
	return (rx_data);
}

int spi_read_status(){
	*(volatile uint32_t*)0xFFFF400C = (0x05);															//send data to TX reg (op code)
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	*(volatile uint32_t*)0xFFFF400C = (0xFF);															//send data to TX reg (dummy byte)
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
	int data = *((volatile uint32_t*)(0xFFFF4008));
	
	spi_put((uint32_t*)0xFFFF4000, 0x00);
	int dummy = *((volatile uint16_t*)(0xFFFF4008));																			//read rx register
	
	return data;
}