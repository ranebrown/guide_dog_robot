/*
*	main.c
*/

// standard includes
#include <stdint.h>

// project specific includes
#include "asf.h" // includes everything added with ASF wizard
#include "gpio.h"
#include "usart.h"
#include "spi.h"
#include "twim.h"
#include "spifun.h"
#include "i2cfun.h"
#include "fpga.h"
#include "motor.h"
#include "eic.h"
#include "pwma.h"
#include "genclk.h"
#include "sysclk.h"
#include "nunchuck.h"


//GLOBALS

int tension = 0;

/*Magnetometer axis*/
int mag_x;
int MAG_XP = (uint32_t*)(&mag_x);
int mag_y;
int MAG_YP = (uint32_t*)(&mag_y);
int mag_z;
int MAG_ZP = (uint32_t*)(&mag_z);

extern double *refSpeed_p;
extern double *refSpeed_p2;
extern double *Kp_p;
extern double *Ki_p;
extern double *Kd_p;
extern double *scaler_p;
extern uint32_t *PWMA_base;

/* i2c buffer */
char I2C_BUFFER[32];
char *I2C_BUFFERP = (char*)(&I2C_BUFFER);
int state = 0;			// duty cycle


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
	WRITE_MR(0x000E0011);
	//unsigned int file_length = program_flash(); //comment this in to upload bin file
	WRITE_MR(0x000E0051);
	//echo_back_flash_data(file_length);
	
	usart_write_line(USART3_BASE_ADDR, "reading file length from flash...\r\n");
	int fl = spi_read_flash4(0); 
	send_binary_to_terminal(fl>>24);
	send_binary_to_terminal(fl>>16); 
	send_binary_to_terminal(fl>>8); 
	send_binary_to_terminal(fl);
	usart_write_line(USART3_BASE_ADDR, "\r\n");
	
	usart_write_line(USART3_BASE_ADDR, "Configuring FPGA...\r\n");
	int is_not_configured = configure_fpga2(fl);
	if (is_not_configured == 1){
		usart_write_line(USART3_BASE_ADDR, "Configuration Failed\r\n");
	} else {
		usart_write_line(USART3_BASE_ADDR, "Configuration Complete!\r\n");
	}
	
	// i2c configuration
	gpio_enable_gpio_pin(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PB04);
	gpio_enable_module_pin(AVR32_TWIMS1_TWCK_0_1_PIN, 6);						//enable i2c clk
	gpio_enable_module_pin(AVR32_TWIMS1_TWD_0_PIN, AVR32_TWIMS1_TWD_0_FUNCTION); //enable data
	i2c_init();
	
	WRITE_MR(0x00070011);															//set CS3 (TEMPORARY)
	
	// initialize ADC
	set_adc_ce(2, 0,adc_ce_reg_pointer);
	//set_adc_sample_rate(1,50);
	set_adc_sample_rate(2,40000);
	set_adc_ce(2, 1,adc_ce_reg_pointer);
	
	
	//char firm = getMotorFirmwareVersion();
	//usart_write_line(USART3_BASE_ADDR, "\r\n");
	//usart_write_line(USART3_BASE_ADDR, "Motor firmware version: ");
	//send_binary_to_terminal(firm);
	//usart_write_line(USART3_BASE_ADDR, "\r\n");
	int d = 0;
	int stop = 0;
	initInterrupt();
	
	int angle[91];          // Angle Array
	int pos = 0;			// servo Angle
	setPWM(4.75);
	
	// nunchuck initialization
	char nunchuckBuff[6] = {0};
	uint8_t c = 0, z = 0;		// buttons
	uint8_t xJoy = 0, yJoy=0;	// joystick
	initNunchuck();
	
	setPWM(50);
		
	// main loop
	while (1) {
		
		//int uart_data = usart_getchar(USART3_BASE_ADDR) & 0xFF;						//wait for uart data
		//spi_write_FPGA(0, 0x80, (uart_data-48));
		
		//int adc2_1 =  (spi_read_FPGA(0, 0x04)) & 0xFF;
		//send_binary_to_terminal(adc2_1);
		//usart_write_line(USART3_BASE_ADDR, "\r\n");
		
		//int uart_data = 1;
		//
		//if (uart_data == 'w'){
			//*refSpeed_p += .1;
			//*refSpeed_p2 += .1;
			//usart_write_line(USART3_BASE_ADDR, "ref speed + .1\r\n");
		//} else if (uart_data == 's'){
			//*refSpeed_p -= .1;
			//*refSpeed_p2 -= .1;
			//usart_write_line(USART3_BASE_ADDR, "ref speed - .1\r\n");
		//} else if (uart_data == '0'){
			//*refSpeed_p = 0;
			//*refSpeed_p2 = 0;
			//Disable_global_interrupt();
			//usart_write_line(USART3_BASE_ADDR, "Motor off\r\n");
			//setMotorSpeeds(0,0);
		//} else if (uart_data == '1'){
			//*refSpeed_p = 1.5;
			//*refSpeed_p2 = 1.5;
			//Enable_global_interrupt();
			//usart_write_line(USART3_BASE_ADDR, "Motor on\r\n");
		//} else if (uart_data == 'p'){
			//*scaler_p += 100;
			//usart_write_line(USART3_BASE_ADDR, "Scaler: ");
			//send_binary_to_terminal((int)(*scaler_p)>>8);
			//send_binary_to_terminal((int)(*scaler_p));
			//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//} else if (uart_data == 'o'){
			//*scaler_p -= 100;
			//usart_write_line(USART3_BASE_ADDR, "Scaler: ");
			//send_binary_to_terminal((int)(*scaler_p)>>8);
			//send_binary_to_terminal((int)(*scaler_p));
			//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//} else if (uart_data == 'l'){
			//*Kp_p += 10;
			//usart_write_line(USART3_BASE_ADDR, "Kp: ");
			//send_binary_to_terminal((int)(*Kp_p)>>8);
			//send_binary_to_terminal((int)(*Kp_p));
			//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//} else if (uart_data == 'k'){
			//*Kp_p -= 10;
			//usart_write_line(USART3_BASE_ADDR, "Kp: ");
			//send_binary_to_terminal((int)(*Kp_p)>>8);
			//send_binary_to_terminal((int)(*Kp_p));
			//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//}else if (uart_data == '9'){
			//// get data
			//getNunchuckData(nunchuckBuff);
			//// decode button data
			//getNunchuckButtons(nunchuckBuff, &z, &c);
			//// get joystick data
			//getNunchuckJoy(nunchuckBuff, &xJoy, &yJoy);
			//// print results to terminal
			//usart_write_line(USART3_BASE_ADDR, "z button:\t");
			//send_binary_to_terminal(z);
			//usart_write_line(USART3_BASE_ADDR, "\t");
			//usart_write_line(USART3_BASE_ADDR, "c button:\t");
			//send_binary_to_terminal(c);
			//usart_write_line(USART3_BASE_ADDR, "\r\n");
			//usart_write_line(USART3_BASE_ADDR, "\t");
			//send_binary_to_terminal(xJoy);
			//usart_write_line(USART3_BASE_ADDR, "\t");
			//send_binary_to_terminal(yJoy);
			//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//
		//}
		
		for(d=0;d<10000;d++); // need delay or else read freezes microcontroller
		// get data
		getNunchuckData(nunchuckBuff);
		getNunchuckJoy(nunchuckBuff, &xJoy, &yJoy);
		//send_binary_to_terminal(yJoy);
		//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//if(!tension)
		//{
		if(!stop)
		{
			setMotorBrakes(0,0);
			if(yJoy > 200)
			{
				//setMotorSpeeds( 50, 50);
				*refSpeed_p = 1.2;
				*refSpeed_p2 = 1.2;
			}
			if(yJoy <= 200)
			{
				//setMotorSpeeds( 0, 0);
				//settMotorBrakes( 0, 0);
				
				*refSpeed_p = 0;
				*refSpeed_p2 = 0;
			}
		}			
		//}
	
		 //  sweep servo
		setPWM(45);
		for(d=0; d<1000000; d++){
			if(d%10000==0)
			{
				int adc2_1 =  (spi_read_FPGA(0, 0x04)) & 0xFF;
				//send_binary_to_terminal(adc2_1);
				//usart_write_line(USART3_BASE_ADDR, "\r\n");
				if(adc2_1>50)
				{
					*refSpeed_p = 0;
					*refSpeed_p2 = 0;
					stop = 1;
					setMotorBrakes(127,127);
				}
				else
				{
					stop = 0;
				}
			}							

		}
		setPWM(50);
		for(d=0; d<1000000; d++)
		{
			if(d%10000==0)
			{
				int adc2_1 =  (spi_read_FPGA(0, 0x04)) & 0xFF;
				//send_binary_to_terminal(adc2_1);
				//usart_write_line(USART3_BASE_ADDR, "\r\n");
				if(adc2_1>50)
				{
					*refSpeed_p = 0;
					*refSpeed_p2 = 0;
					stop = 0;
					setMotorBrakes(127,127);
				}
				else
				{
					stop = 0;
				}
			}
		}
		
		//if(stop==1)
		//{
			//*refSpeed_p = 0;
			//*refSpeed_p2 = 0;	
		//}
		


		// Scan 90 degrees and store distance and angle readings
		//for(pos = 0; pos <= 90; pos += 1)
		//{
		//	uart_data2 = usart_getchar(USART3_BASE_ADDR) & 0xFF;						//wait for uart data
		//	usart_putchar(USART3_BASE_ADDR,uart_data2);
		//	angle[pos] = pos;
		//	pwma_set_channels_value(&AVR32_PWMA,(1<<AVR32_PWMA_4_PIN),);
		//}

			//if(state == 0){
				//setPWM(4.725);
				//state = 1;
			//} else if (state == 1){
				//setPWM(9.175);
				//state = 0;
			//}
		


		//usart_write_line(USART3_BASE_ADDR, "\r\n");
		//usart_putchar(USART3_BASE_ADDR, uart_data);									//echo back data
		//gpio_enable_gpio_pin(AVR32_PIN_PA17);
		//spi_write_FPGA(0, 0x80, (uart_data-48));									//write data to LED control reg
		
		//usart_write_line(USART3_BASE_ADDR, "\r\n");
		
		/*
		int left =  spi_read_FPGA(0, 0x00);
		usart_write_line(USART3_BASE_ADDR, "Left Wheel: ");
		send_binary_to_terminal(left);
		usart_write_line(USART3_BASE_ADDR, "\r\n");
		
		int right =  spi_read_FPGA(0, 0x01);
		usart_write_line(USART3_BASE_ADDR, "Right Wheel: ");
		send_binary_to_terminal(right);
		usart_write_line(USART3_BASE_ADDR, "\r\n");
		
		int adc1_1 =  spi_read_FPGA(0, 0x02);
		usart_write_line(USART3_BASE_ADDR, "ADC1 channel 1: ");
		send_binary_to_terminal(adc1_1);
		usart_write_line(USART3_BASE_ADDR, "\r\n");
		
		int adc1_2 =  spi_read_FPGA(0, 0x03);
		usart_write_line(USART3_BASE_ADDR, "ADC1 channel 2: ");
		send_binary_to_terminal(adc1_2);
		usart_write_line(USART3_BASE_ADDR, "\r\n");
		
		//char i2c_data[4] = {0x55,0x54,0x53,0x52};
		//i2c_write(0x08,&i2c_data,4);
		*/
		//usart_putchar(USART2_BASE_ADDR, 0x0D);
		//int lidar = getLidar(I2C_BUFFERP);
		//usart_write_line(USART3_BASE_ADDR, "Lidar val: ");
		//send_binary_to_terminal(lidar>>8);
		//send_binary_to_terminal(lidar);	
		//usart_write_line(USART3_BASE_ADDR, "\r\n");	
		
		
	}
}
