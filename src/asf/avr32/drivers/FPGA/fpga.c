
#include "fpga.h"

//returns the length of the file i - 4 -5
unsigned int program_flash(void){
	
	//erase the whole chip
	spi_chip_erase();
	
	//first word (0x00000) will be the length in bytes of the config file
	unsigned int i = 4;										//keeps track of how many bytes are programmed
	int char_buffer_count = 0;									//used as a counter for a fifo that holds the last 6 characters
	char done_sequence_buffer[6] = {'x','d','o','n','e','x'};	//by typing "xdonex" into the terminal, this will signal that the binary file is finished programming
	char char_buffer[6];										//stores the last 6 characters in the terminal
	int done_flag = 0;

	while (1) {
		// wait for UART data
		int uart_data = usart_getchar((uint32_t*)0xFFFF3C00);
	
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );				//wait for Tx reg to empty
		while(1){
			int flash_status_reg = spi_read_status();																			//read status register to make sure a write is not in progress
			while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );			//wait fot Tx reg to empty
			if ((flash_status_reg & 1) == 0) break;																				//check if busy bit = 0. if so a write to flash can be done
		}
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );				//wait for Tx reg to empty
		spi_write_flash(i,uart_data);																							//write uart data to flash
	
		//this code checks if the programming the flash chip was completed
		//FROM HERE
		char_buffer[char_buffer_count] = (char)uart_data;			//store the current character in the fifo
		int temp_count,loop_count;
		temp_count = char_buffer_count + 1;							//increment by one because we are assuming that this was the last character in the sequence
		for (loop_count = 0; loop_count < 6; ++loop_count){
			if (temp_count > 5) temp_count = 0;						//make sure the counter doesnt go outside of the fifo size
				if (char_buffer[temp_count] == done_sequence_buffer[loop_count]){
					temp_count++;
					continue;
				} else {
					done_flag = 1;
					break;
				}
			}
			if (done_flag == 0) break;
			done_flag = 0;
			char_buffer_count++;
			if (char_buffer_count > 5){
				char_buffer_count = 0;
			}
			//TO HERE
	
			i++;																	// increment adress
			if (i > 0x7FFFF){														// make sure there is not a memory overflow
			usart_write_line((uint32_t*)0xFFFF3C00, "MEM OVERFLOW!\r\n");
			i = 0;
			while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
			spi_chip_erase();
		}
	}
	unsigned int file_length = i - 9;											//subtract 4 for the file length, subtract 5 for the "xdonex" sequence. 4+5=9
	
	//STORE THE FILE LENGTH
	int j;
	for (j = 0; j < 4; j++){
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
		while(1){
			int flash_status_reg = spi_read_status();
			while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
			if ((flash_status_reg & 1) == 0) break;
		}
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
		spi_write_flash(j,(file_length>>(24-(8*j))));
	}
	return file_length;
}



int echo_back_flash_data(int len){
	int i;
	for (i = 4; i < (len+4); i++){
		while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
		unsigned int flash_data = spi_read_flash(i);
		send_hex_to_terminal(flash_data);
		//send_binary_to_terminal(flash_data);
		//usart_putchar((uint32_t*)0xFFFF3C00, flash_data);
	}
	usart_putchar((uint32_t*)0xFFFF3C00, 0x0D);
	usart_putchar((uint32_t*)0xFFFF3C00, 0x0A);
}

int configure_fpga(unsigned int file_length){
	//CONFIGURATION CODE
	int i,j,k;
	int delay_len = 1;
	
	uint32_t config_file_start_addr = 4;
	
	// Configuration Reset
	gpio_clr_gpio_pin(AVR32_PIN_PA11);											//de-assert program_b to reset device
	//gpio_tgl_gpio_pin(AVR32_PIN_PA20);											//used as a delay

	gpio_set_gpio_pin(AVR32_PIN_PA11);											//assert program_b to start configuration process
	
	
	// Wait for Device Initialization
	while(gpio_get_pin_value(AVR32_PIN_PA22) == 0);							// wait until init_b is asserted and then send bit stream

	// Configuration (Bitstream) Load
	int addr, data8;
	gpio_clr_gpio_pin(AVR32_PIN_PB05);											//de-assert CCLK
	for (i = 0; i < file_length; i++) {
		//usart_putchar((uint32_t*)0xFFFF3C00, 'f');
		addr = config_file_start_addr + i;
		data8 = spi_read_flash(addr);											//read next byte, (clock is low, reading acts as a delay)
		for (j = 7; j >= 0; --j){
			uint32_t bitval = ((data8 >> j) & 1);
			if (bitval){
				gpio_set_gpio_pin(AVR32_PIN_PA05);								//if bit is 1 set DIN to 1
				//usart_putchar((uint32_t*)0xFFFF3C00, '1');
			} else {
				gpio_clr_gpio_pin(AVR32_PIN_PA05);								//if bit is 0 set DIN to 0
				//usart_putchar((uint32_t*)0xFFFF3C00, '0');
			}
			gpio_set_gpio_pin(AVR32_PIN_PB05);									//assert CCLK
			gpio_clr_gpio_pin(AVR32_PIN_PB05);									//de-assert CCLK
		}
	}
	
	// Check INIT_B
	if (gpio_get_pin_value(AVR32_PIN_PA22) == 0) {
		usart_write_line((uint32_t*)0xFFFF3C00, "ERROR CONFIG 1!\r\n");
		return 1;
	}
	// Wait for DONE to assert
	int timeout = 0xFF;
	while(gpio_get_pin_value(AVR32_PIN_PB10) == 0){
		timeout--;
		if (timeout == 0){
			usart_write_line((uint32_t*)0xFFFF3C00, "ERROR CONFIG 2!\r\n");
			break;
		}
	}
	// Compensate for Special Startup Conditions
	for (i = 0; i < delay_len; i++);
	return 0;
}

int spi_write_FPGA(int cs, int addr, int data){
	
	int spi_data, rx_data, spi_addr;
	if (cs == 0){
		spi_data = data;// | 0x000B0000;
		spi_addr = addr;// | 0x000B0000;
	} else {
		spi_data = data;// | 0x00070000;
		spi_addr = addr;// | 0x000B0000;
	}
	*(volatile uint32_t*)0xFFFF400C = spi_addr;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	*(volatile uint32_t*)0xFFFF400C = spi_data;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	//while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK ); //wait for tx buffer to empty
}

int spi_read_FPGA(int cs, int addr){
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
	int rx_data;
	*(volatile uint32_t*)0xFFFF400C = addr;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	*(volatile uint32_t*)0xFFFF400C = 0xFF;	//send data to TX reg
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_TXEMPTY_MASK) != AVR32_SPI_SR_TXEMPTY_MASK );
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK ); //wait for rx buffer to be full
	rx_data = *((volatile uint32_t*)(0xFFFF4008));
	
	spi_put((uint32_t*)0xFFFF4000, 0x00);
	int dummy = *((volatile uint16_t*)(0xFFFF4008));
	
	return rx_data;
}

int set_adc_sample_rate(int adc_num,int sample_rate_Hz){
	int clk_rate = 1562500;
	int period = clk_rate/sample_rate_Hz;
	int msb = (period & 0xFF00) >> 8;
	int lsb = (period & 0xFF);
	send_binary_to_terminal(msb);
	send_binary_to_terminal(lsb);
	spi_write_FPGA(0, (0x81 + 2*adc_num - 1),msb);
	spi_write_FPGA(0,(0x82 + 2*adc_num - 1),lsb);
}

int set_adc_ce(int adc_num, int on,int *current_val){
	if (on == 0){
		*current_val |= adc_num;
	} else if (on == 1){
		*current_val &= ~adc_num;
	}
	send_binary_to_terminal(*current_val);
	spi_write_FPGA(0,0x81,*current_val);
}

void send_binary_to_terminal(unsigned int data){
	int i;
	for(i = 7; i >= 0; i--){
		int bit = (data >> i) & 1;
		if (bit){
			usart_putchar((uint32_t*)0xFFFF3C00, 0x31);		//"1"
		} else {
			usart_putchar((uint32_t*)0xFFFF3C00, 0x30);		//"0"
		}
	}
	usart_putchar((uint32_t*)0xFFFF3C00, 0x20);				//space
}

void send_hex_to_terminal(unsigned int data){
	int data1 = (data >> 4) & 0x0F;
	int data0 = data & 0x0F;
	if (data1 < 10){
		usart_putchar((uint32_t*)0xFFFF3C00, (0x30 + data1));
	} else {
		usart_putchar((uint32_t*)0xFFFF3C00, (0x41-0xA+data1));
	}
	
	if (data0 < 10){
		usart_putchar((uint32_t*)0xFFFF3C00, (0x30 + data0));
	} else {
		usart_putchar((uint32_t*)0xFFFF3C00, (0x41-0xA+data0));
	}
}