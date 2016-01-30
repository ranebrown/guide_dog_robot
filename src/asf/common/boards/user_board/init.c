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

// uart config parameters
static const usart_options_t USART_OPTIONS = {
	.baudrate = 115200,
	.charlength = 8,
	.paritytype = USART_NO_PARITY,
	.stopbits = USART_1_STOPBIT,
	.channelmode = USART_NORMAL_CHMODE
};
static const usart_options_t USART_OPTIONS2 = {
	.baudrate = 9600,
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
void board_init()
{
	// init GPIO
	gpio_enable_gpio_pin(AVR32_PIN_PA15);
	gpio_enable_gpio_pin(AVR32_PIN_PA16);

	// Initial SPI pins
	gpio_enable_module_pin(AVR32_SPI_SCK_0_1_PIN, AVR32_SPI_SCK_0_1_FUNCTION);				//Enable SCK
	gpio_enable_module_pin(AVR32_SPI_MOSI_0_1_PIN, AVR32_SPI_MOSI_0_1_FUNCTION);			//Enable MOSI
	gpio_enable_module_pin(AVR32_SPI_MISO_0_1_PIN, AVR32_SPI_MISO_0_1_FUNCTION);			//Enable MISO
	gpio_enable_module_pin(AVR32_SPI_NPCS_0_1_PIN, AVR32_SPI_NPCS_0_1_FUNCTION);			//Enable CS0 (FLASH)
	gpio_enable_module_pin(AVR32_SPI_NPCS_2_0_PIN, AVR32_SPI_NPCS_2_0_FUNCTION);			//Enable CS2 (FPGA1)
	gpio_enable_module_pin(AVR32_SPI_NPCS_3_0_PIN, AVR32_SPI_NPCS_3_0_FUNCTION);			//Enable CS3 (FPGA2)
	
	// set values for spi registers
	uint32_t *SPI_BASE = (volatile uint32_t*)0xFFFF4000;
	WRITE_MR(0x000E0011);
	WRITE_CS0(0x00000502);
	WRITE_CS3(0x00000502);
	WRITE_CR(0x00000001);

	// Initialize UART pins
	//UART 2 is motor driver
	gpio_enable_module_pin(AVR32_USART2_TXD_0_0_PIN, AVR32_USART2_TXD_0_0_FUNCTION);			//UART 2 TX
	gpio_enable_module_pin(AVR32_USART2_RXD_0_0_PIN, AVR32_USART2_RXD_0_0_FUNCTION);			//UART 2 RX
	uint32_t *USART2_BASE_ADDR = (uint32_t*)0xFFFF3800;
	
	//UART 3 is used to communicate with PC
	gpio_enable_module_pin(AVR32_USART3_TXD_0_1_PIN, AVR32_USART3_TXD_0_1_FUNCTION);			//UART 3 TX
	gpio_enable_module_pin(AVR32_USART3_RXD_0_1_PIN, AVR32_USART3_RXD_0_1_FUNCTION);			//UART 3 RX
	uint32_t *USART3_BASE_ADDR = (uint32_t*)0xFFFF3C00;
	
	// set uart baud rate and options
	usart_init_rs232(USART3_BASE_ADDR, &USART_OPTIONS, 17200000);
	usart_init_rs232(USART2_BASE_ADDR, &USART_OPTIONS2, 17200000);
	
	// initialize i2c pins
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
	 
}

/*
* description: initializes gpio pins
* args: none
* return: none
*/
void initClock() {
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

	// Set the main clock source to be DFLL0.
	pm_set_mclk_source(PM_CLK_SRC_DFLL0);
	
}

/*
* description: outputs clock signal to gpio pin 6
* args: none
* return: none
*/
void gpioClock() {
	// Note: for UC3L devices, the generic clock configurations are handled by the SCIF module.
	// Setup gc to use the DFLL0 as source clock, divisor enabled, apply a division factor.
	// Since the DFLL0 frequency is 96MHz, set the division factor to 2 to have a gclk frequency of 48MHz.
	scif_gc_setup(AVR32_SCIF_GCLK_DFLL0_SSG, SCIF_GCCTRL_DFLL0, AVR32_GC_DIV_CLOCK, 2);

	/* Now enable the generic clock */
	scif_gc_enable(AVR32_SCIF_GCLK_DFLL0_SSG);
	
	/* Set the GCLOCK function to the GPIO pin */
	gpio_enable_module_pin(AVR32_SCIF_GCLK_1_0_PIN, AVR32_SCIF_GCLK_1_0_FUNCTION);
}