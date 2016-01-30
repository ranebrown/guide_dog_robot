/*
*	main.c
*/

// standard includes
#include <stdint.h>

// project specific includes
#include <asf.h> // includes everything added with ASF wizard
#include "gpio.h"
#include "usart.h"
#include "spi.h"
#include "twim.h"
#include "spifun.h"
#include "i2cfun.h"
#include "fpga.h"

int main (void)
{
	// variables
	uint32_t *USART3_BASE_ADDR = (uint32_t*)0xFFFF3C00;
	uint32_t *USART2_BASE_ADDR = (uint32_t*)0xFFFF3800;
	uint32_t *SPI_BASE_MAIN = (uint32_t*)0xFFFF4000;
	uint32_t *TWIM1_BASE = (uint32_t *)0xFFFF4400;
	int flash_data = 0x65;
	int *flash_data_pointer = &flash_data;
	int clr_rx;
	int adc_ce_reg = 0x00;
	int adc_ce_reg_pointer = &adc_ce_reg;
	int i2c_dummy = 0x55;
	int *i2c_dummy_p = &i2c_dummy;
	
	// initialization
	initClock();			// set clock to 48MHz
	board_init();			// mainly for communication profiles
	gpioClock();			// output clock to gpio pin 6
		
	//enable write for flash chip
	spi_put(SPI_BASE_MAIN, 0x06);
	clr_rx = *((volatile uint16_t*)(0xFFFF4008));
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK );
	spi_unlock_flash();
	while ( (*((volatile uint32_t*)(0xFFFF4010)) & AVR32_SPI_SR_RDRF_MASK) != AVR32_SPI_SR_RDRF_MASK );
	
	
	// configure FPGA
	//unsigned int file_length = program_flash();
	//echo_back_flash_data(file_length);
	usart_write_line(USART3_BASE_ADDR, "reading file length from flash...\r\n");
	uint32_t file_length00 = 0;
	uint32_t file_length3,file_length2,file_length1,file_length0;
	file_length3 = (uint32_t)spi_read_flash(0) & 0x000000FF;						//program_flash();
	send_binary_to_terminal(file_length3);
	file_length2 = (uint32_t)spi_read_flash(1) & 0x000000FF;
	send_binary_to_terminal(file_length2);
	file_length1 = (uint32_t)spi_read_flash(2) & 0x000000FF;
	send_binary_to_terminal(file_length1);
	file_length0 = (uint32_t)spi_read_flash(3) & 0x000000FF;
	send_binary_to_terminal(file_length0);
	file_length00 = file_length3<<24 | file_length2<<16 | file_length1<<8 | file_length0<<0 | 0;
	usart_putchar(USART3_BASE_ADDR, 0x0D);
	usart_putchar(USART3_BASE_ADDR, 0x0A);
	usart_write_line(USART3_BASE_ADDR, "Configuring FPGA...\r\n");
	int is_not_configured = configure_fpga(file_length00);
	if (is_not_configured == 1){
		usart_write_line(USART3_BASE_ADDR, "Configuration Failed\r\n");
	} else {
		usart_write_line(USART3_BASE_ADDR, "Configuration Complete!\r\n");
	}
	
	// i2c configuration
	gpio_enable_gpio_pin(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PB04);
	//gpio_enable_module_pin(AVR32_TWIMS0_TWD_0_2_PIN, 6);
	//gpio_enable_module_pin(AVR32_TWIMS0_TWCK_0_2_PIN, AVR32_TWIMS0_TWCK_0_2_FUNCTION);
	gpio_enable_module_pin(AVR32_TWIMS1_TWCK_0_1_PIN, 6);			//enable i2c clk
	gpio_enable_module_pin(AVR32_TWIMS1_TWD_0_PIN, AVR32_TWIMS1_TWD_0_FUNCTION);
	// reading registers for debugging
	send_binary_to_terminal(*((volatile uint32_t*)(0xFFFF481C))>>16);
	send_binary_to_terminal(*((volatile uint32_t*)(0xFFFF481C))>>8);
	send_binary_to_terminal(*((volatile uint32_t*)(0xFFFF481C)));
	i2c_init();
	send_binary_to_terminal(*((volatile uint32_t*)(0xFFFF481C))>>16);
	send_binary_to_terminal(*((volatile uint32_t*)(0xFFFF481C))>>8);
	send_binary_to_terminal(*((volatile uint32_t*)(0xFFFF481C)));
	*((volatile uint32_t*)(0xFFFF481C)) = 0x400;
	WRITE_MR(0x00070011);
	
	// initialize ADC
	set_adc_ce(3, 0,adc_ce_reg_pointer);															//set CS3 (TEMPORARY)
	set_adc_sample_rate(1,400);
	set_adc_sample_rate(2,400);
	set_adc_ce(3, 1,adc_ce_reg_pointer);
	
	// main loop
	while (1) {
		int uart_data = usart_getchar(USART3_BASE_ADDR) & 0xFF;						//wait for uart data
		usart_putchar(USART3_BASE_ADDR, uart_data);									//echo back data
		//gpio_enable_gpio_pin(AVR32_PIN_PA17);
		spi_write_FPGA(0, 0x80, (uart_data-48));									//write data to LED control reg
		//usart_putchar(USART2_BASE_ADDR, 0x0D);
		int lidar = getLidar();
		//send_binary_to_terminal(lidar>>8);
		//send_binary_to_terminal(lidar);		
		//gpio_tgl_gpio_pin(AVR32_PIN_PA17);					
	}
}
