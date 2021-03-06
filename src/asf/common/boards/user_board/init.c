/*
*	init.c
*	description: contains functions used to initialize PCB
*/

#include <asf.h>
#include <board.h>
#include <conf_board.h>
#include "uc3l0256.h"
#include "gpio.h"
#include "usart.h"
#include "spi.h"
#include "scif_uc3l.h"
#include "twim.h"
#include "eic.h"
#include "fpga.h"
#include "motor.h"

extern int tension;

//PWM modules
uint32_t  *PWMA_base = (volatile uint32_t*)0xFFFF5400;
//right wheel
double x[2];
double y[2];

//left wheel
double x2[2];
double y2[2];

double Kp = 10;
double Ki = 150;
double Kd = 20;

//right speed
double refSpeed = 0;
double *refSpeed_p = (int*)&refSpeed;

//left speed
double refSpeed2 = 0;
double *refSpeed_p2 = (int*)&refSpeed2;

// uart config parameters
static const usart_options_t USART_OPTIONS = {
	.baudrate = 115200,
	.charlength = 8,
	.paritytype = USART_NO_PARITY,
	.stopbits = USART_1_STOPBIT,
	.channelmode = USART_NORMAL_CHMODE
};
static const usart_options_t USART_OPTIONS2 = {
	.baudrate = 115200,
	.charlength = 8,
	.paritytype = USART_NO_PARITY,
	.stopbits = USART_1_STOPBIT,
	.channelmode = USART_NORMAL_CHMODE
};

/*
* description: initializes PCB and peripherals such as uart, i2c, spi
* args: none
* return: none
*/
void board_init(void)
{
	// init GPIO
	gpio_enable_gpio_pin(AVR32_PIN_PA15);
	gpio_enable_gpio_pin(AVR32_PIN_PA16);
	
	//TODO: make these input pins
	gpio_enable_gpio_pin(AVR32_PIN_PB11);			//gpio11 left wheel direction
	gpio_enable_gpio_pin(AVR32_PIN_PA10);			//gpio12 right wheel direction

	//PWM Setup
	//pwma_config_and_enable(PWMA_base,(1 << AVR32_PWMA_4_PIN) 255, 0);	//configure PWM channel with a top of 255 and a starting duty cycle of 0
	//gpio_enable_module_pin(AVR32_PWMA_4_PIN ,AVR32_PWMA_4_FUNCTION); //enable servo PWM pin 
	
	// Initial SPI pins
	gpio_enable_module_pin(AVR32_SPI_SCK_0_1_PIN, AVR32_SPI_SCK_0_1_FUNCTION);				//Enable SCK
	gpio_enable_module_pin(AVR32_SPI_MOSI_0_1_PIN, AVR32_SPI_MOSI_0_1_FUNCTION);			//Enable MOSI
	gpio_enable_module_pin(AVR32_SPI_MISO_0_1_PIN, AVR32_SPI_MISO_0_1_FUNCTION);			//Enable MISO
	gpio_enable_module_pin(AVR32_SPI_NPCS_0_1_PIN, AVR32_SPI_NPCS_0_1_FUNCTION);			//Enable CS0 (FLASH)
	gpio_enable_module_pin(AVR32_SPI_NPCS_2_0_PIN, AVR32_SPI_NPCS_2_0_FUNCTION);			//Enable CS2 (FPGA1)
	gpio_enable_module_pin(AVR32_SPI_NPCS_3_0_PIN, AVR32_SPI_NPCS_3_0_FUNCTION);			//Enable CS3 (FPGA2)
	
	
	// set values for spi registers
	uint32_t *SPI_BASE = (volatile uint32_t*)0xFFFF4000;
	WRITE_MR(0x000E0051);
	//WRITE_MR(0x000E0011);
	WRITE_CS0(0x00000502);
	WRITE_CS3(0x00000502);
	WRITE_CR(0x00000001);

	// Initialize UART pins
	//UART 2 is motor driver
	//gpio_enable_module_pin(AVR32_USART2_TXD_0_0_PIN, AVR32_USART2_TXD_0_0_FUNCTION);			//UART 2 TX
	gpio_enable_module_pin(AVR32_USART2_TXD_0_1_PIN, AVR32_USART2_TXD_0_1_FUNCTION);
	gpio_enable_module_pin(AVR32_USART2_RXD_0_0_PIN, AVR32_USART2_RXD_0_0_FUNCTION);			//UART 2 RX
	uint32_t *USART2_BASE_ADDR = (uint32_t*)0xFFFF3800;
	
	//UART 3 is used to communicate with PC
	gpio_enable_module_pin(AVR32_USART3_TXD_0_1_PIN, AVR32_USART3_TXD_0_1_FUNCTION);			//UART 3 TX
	gpio_enable_module_pin(AVR32_USART3_RXD_0_1_PIN, AVR32_USART3_RXD_0_1_FUNCTION);			//UART 3 RX
	uint32_t *USART3_BASE_ADDR = (uint32_t*)0xFFFF3C00;
	
	// set uart baud rate and options
	usart_init_rs232(USART3_BASE_ADDR, &USART_OPTIONS, 19750000);
	usart_init_rs232(USART2_BASE_ADDR, &USART_OPTIONS2, 19750000);
	
	// initialize i2c pins
	gpio_enable_gpio_pin(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PA17);
	gpio_enable_pin_pull_up(AVR32_PIN_PB04);
	gpio_enable_module_pin(AVR32_TWIMS1_TWCK_0_1_PIN, 6);							//enable i2c clk
	gpio_enable_module_pin(AVR32_TWIMS1_TWD_0_PIN, AVR32_TWIMS1_TWD_0_FUNCTION);	//enable i2c data
	
	//Initialize FGPA config pins
	gpio_enable_gpio_pin(AVR32_PIN_PB05);					// init CCLK
	gpio_enable_gpio_pin(AVR32_PIN_PA05);					// init DIN
	gpio_enable_gpio_pin(AVR32_PIN_PA11);					// init PROGRAM_B
	gpio_enable_gpio_pin(AVR32_PIN_PA22);					// init INIT_B
	gpio_enable_gpio_pin(AVR32_PIN_PB10);					// init DONE
	
	//set configuration flags
	uint32_t cclk_config = GPIO_DIR_OUTPUT | GPIO_INIT_LOW;
	uint32_t din_config = GPIO_DIR_OUTPUT | GPIO_INIT_LOW;
	uint32_t program_b_config = GPIO_DIR_OUTPUT | GPIO_INIT_HIGH;
	uint32_t init_b_config = GPIO_DIR_INPUT;
	uint32_t done_config = GPIO_DIR_INPUT;
	
	//set pins as input/output
	gpio_configure_pin(AVR32_PIN_PB05, cclk_config);
	gpio_configure_pin(AVR32_PIN_PA05, din_config);
	gpio_configure_pin(AVR32_PIN_PA11, program_b_config);
	gpio_configure_pin(AVR32_PIN_PA22, init_b_config);
	gpio_configure_pin(AVR32_PIN_PB10, done_config);
	
	// gpio pin for raspberry pi read (computer vision)
	//gpio_enable_gpio_pin(AVR32_PIN_PB09);
	//gpio_configure_pin(AVR32_PIN_PB09, GPIO_DIR_INPUT); 
}

/*
* description: initializes gpio pins
* args: none
* return: none
*/
void initClock(void) {
	scif_dfll_openloop_conf_t dfllconfig ={150,65};
	
	// Configure and start the DFLL0 in open loop mode to generate a frequency of 96MHz.
	scif_dfll0_openloop_start(&dfllconfig);

	// Since our target is to set the CPU&HSB frequency domains to 48MHz, we must
	// set one wait-state and enable the High-speed read mode on the flash controller.
	flashcdw_set_flash_waitstate_and_readmode((48000000UL));

	// Set the CPU clock domain to 48MHz (by applying a division ratio = 2).
	pm_set_clk_domain_div((pm_clk_domain_t)AVR32_PM_CLK_GRP_CPU, PM_CKSEL_DIVRATIO_2);

	// Set the PBA clock domain to 24MHz (by applying a division ratio = 4).
	pm_set_clk_domain_div((pm_clk_domain_t)AVR32_PM_CLK_GRP_PBA, PM_CKSEL_DIVRATIO_4);

	// Set the PBB clock domain to 48MHz (by applying a division ratio = 2).
	pm_set_clk_domain_div((pm_clk_domain_t)AVR32_PM_CLK_GRP_PBB, PM_CKSEL_DIVRATIO_2);
	
	pm_set_clk_domain_div((pm_clk_src_t)PM_CLK_SRC_SLOW, PM_CKSEL_DIVRATIO_2);
	
	
	
	// Set the main clock source to be DFLL0.
	pm_set_mclk_source(PM_CLK_SRC_DFLL0);
	
}

/*
* description: outputs clock signal to gpio pin 6
* args: none
* return: none
*/
void gpioClock(void) {
	// Note: for UC3L devices, the generic clock configurations are handled by the SCIF module.
	// Setup gc to use the DFLL0 as source clock, divisor enabled, apply a division factor.
	// Since the DFLL0 frequency is 96MHz, set the division factor to 2 to have a gclk frequency of 48MHz.
	scif_gc_setup(AVR32_SCIF_GCLK_DFLL0_SSG, SCIF_GCCTRL_DFLL0, AVR32_GC_DIV_CLOCK, 2);

	/* Now enable the generic clock */
	scif_gc_enable(AVR32_SCIF_GCLK_DFLL0_SSG);
	
	/* Set the GCLOCK function to the GPIO pin */
	/* update: this pin is now used for uart2 TX*/
	//gpio_enable_module_pin(AVR32_SCIF_GCLK_1_0_PIN, AVR32_SCIF_GCLK_1_0_FUNCTION);
}

static void eicHandler_motors(void)
{
	if (eic_is_interrupt_line_pending(&AVR32_EIC, 2))
		eic_clear_interrupt_line(&AVR32_EIC, 2);
		
	//motor constants
	double Ts = 1/47.5;
	
	double K1 = .09;//(*Kp_p+*Ki_p*Ts/2+2*(*Kd_p)/Ts)/(*scaler_p);
	double K2 = -.13;//(*Ki_p*Ts-4*(*Kd_p)/Ts)/(*scaler_p);
	double K3 = .065;//(-(*Kp_p)+*Ki_p*Ts/2+2*(*Kd_p)/Ts)/(*scaler_p);
	
	//right wheel speed
	char enc_speed =  spi_read_FPGA(0, 0x01);
	//left wheel speed
	char enc_speed2 = spi_read_FPGA(0, 0x00);
	
	//compensate for spi error
	if (enc_speed == 0xff) enc_speed = 0;
	if (enc_speed2 == 0xff) enc_speed2 = 0;
	
	//right wheel tf
	double ref_Hz = ((*refSpeed_p)*17.6/14.842*1124/2800*115);
	double xin = ref_Hz - (double)enc_speed;
	double yout = (y[1] + K1*xin + K2*x[0] + K3*x[1] );
	
	//left wheel tf
	double ref_Hz2 = ((*refSpeed_p2)*17.6/14.842*1124/2800*115);
	double xin2 = ref_Hz2 - (double)enc_speed2;
	double yout2 = ( y2[1] + K1*xin2 + K2*x2[0] + K3*x2[1] );
	
	//make sure value doesn't go outside of range
	if (yout > 127){
		yout = 127;
	} else if (yout < -127){
		yout = -127;
	}
	
	if (yout2 > 127){
		yout2 = 127;
	} else if (yout2 < -127){
		yout2 = -127;
	}
	
	if(refSpeed == 0 && refSpeed2 == 0 && (xin == 0 || xin2 == 0))
	{
		x[1] = 0;
		x[0] = 0;
		y[1] = 0;
		y[0] = 0;
		x2[1] = 0;
		x2[0] = 0;
		y2[1] = 0;
		y2[0] = 0;
		setMotorSpeeds(0,0);
	}
	
	//update old values right side
	x[1] = x[0];
	x[0] = xin;
	y[1] = y[0];
	y[0] = yout;
	
	//update old values left side
	x2[1] = x2[0];
	x2[0] = xin2;
	y2[1] = y2[0];
	y2[0] = yout2;
	
	setMotorSpeeds((signed char)yout,(signed char)(yout2));
	
	//if (eic_is_interrupt_line_pending(&AVR32_EIC, 2)){
		////send_binary_to_terminal((char)enc_speed);
		////send_binary_to_terminal((signed char)xin);
		////send_binary_to_terminal((signed char)yout);
		//send_binary_to_terminal(adc2_2);
		//usart_write_line((uint32_t*)0xFFFF3C00, "\r\n");
	//}		
	//else{
		////send_binary_to_terminal((char)enc_speed);
		////send_binary_to_terminal((signed char)xin);
		////send_binary_to_terminal((signed char)yout);
		//send_binary_to_terminal(adc2_2);
		//usart_write_line((uint32_t*)0xFFFF3C00, "\r\n");
	//}		
	return;
}

static void eicHandler_CV(void)
{
	// computer vision pin AVR32_PIN_PB09 = 41
	
	// clear interrupt line
	if (eic_is_interrupt_line_pending(&AVR32_EIC, 3))
	// interrupt vector addr, line number
	eic_clear_interrupt_line(&AVR32_EIC, 3);
	
	usart_write_line((uint32_t*)0xFFFF3C00, "stop sign \r\n");
}

/*
 * initializes interrupt
 * param eic_options = structure containing interrupt settings
 * return 0 success, 1 else
 */
void initInterrupt()
{	
	// encoders gpio3 = PA13 -> EIC[2]
	// computer vision pin AVR32_PIN_PB09 = 41 -> EIC[3]
		
	// structure for interrupt options motor encoders
	eic_options_t eic_options[2];
		
	// motor options
	eic_options[0].eic_mode		= EIC_MODE_EDGE_TRIGGERED;
	eic_options[0].eic_edge		= EIC_EDGE_RISING_EDGE;
	eic_options[0].eic_filter	= EIC_FILTER_ENABLED;
	eic_options[0].eic_async	= EIC_SYNCH_MODE;			// interrupt is synchronized to the clock
	eic_options[0].eic_line		= 2;						// Choose External Interrupt Controller Line
		
	// computer vision options
	eic_options[1].eic_mode			= EIC_MODE_EDGE_TRIGGERED;
	eic_options[1].eic_edge			= EIC_EDGE_RISING_EDGE;
	eic_options[1].eic_filter		= EIC_FILTER_ENABLED;
	eic_options[1].eic_async		= EIC_SYNCH_MODE;			// interrupt is synchronized to the clock
	eic_options[1].eic_line			= 1;						// Choose External Interrupt Controller Line
		
	// map gpio pins to be used as external interrupts
	gpio_enable_module_pin(AVR32_EIC_EXTINT_2_0_PIN, AVR32_EIC_EXTINT_2_0_FUNCTION);
	gpio_enable_module_pin(AVR32_EIC_EXTINT_3_1_PIN, AVR32_EIC_EXTINT_3_1_FUNCTION);
		
	// Disable all interrupts.
	Disable_global_interrupt();

	// Initialize interrupt vectors.
	INTC_init_interrupts();
	
	// map the interrupt handler (function) to the interrupt controller with a given priority 
	// !! check table in datasheet pg 9 for correct EIC[n]
	INTC_register_interrupt(&eicHandler_motors, AVR32_EIC_IRQ_2, AVR32_INTC_INTLEVEL_INT1);
	INTC_register_interrupt(&eicHandler_CV, AVR32_EIC_IRQ_3, AVR32_INTC_INTLEVEL_INT1);
		 
	// Init the EIC controller with the options
	eic_init(&AVR32_EIC, eic_options, 2);
	
	// Enable External Interrupt Controller Line corresponds to EIC[n]
	eic_enable_line(&AVR32_EIC, 2);
	eic_enable_line(&AVR32_EIC, 3);
	eic_enable_interrupt_line(&AVR32_EIC, 2);
	eic_enable_interrupt_line(&AVR32_EIC, 3);
		
	// Enable all interrupts.
	Enable_global_interrupt();
}
