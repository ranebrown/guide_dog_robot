#ifndef _FPGA_
#define _FPGA_
#include <stdint.h>
#include "asf/avr32/drivers/spifun/spifun.h"
#include "asf/avr32/drivers/usart/usart.h"
#include "asf/avr32/drivers/gpio/gpio.h"



unsigned int program_flash();


int echo_back_flash_data(int len);


int configure_fpga(unsigned int file_length);


int spi_write_FPGA(int cs, int addr, int data);


int spi_read_FPGA(int cs, int addr);


int set_adc_sample_rate(int adc_num,int sample_rate_Hz);


int set_adc_ce(int adc_num, int on,int *current_val);


void send_binary_to_terminal(unsigned int data);


void send_hex_to_terminal(unsigned int data);



#endif