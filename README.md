# Guide Dog Robot Software
This is an Atmel Studio project that contains the software for the guide dog robot project. All source files are run on a custom PCB that utilizes a 32 bit microcontroller in conjunction with a FPGA.

## General Information
FPGA modules built using Xilinx Design Suite

PCB programmed with AVR JTAGICE XPII

**Microcontroller:** Atmel 32 Bit microcontroller part #: AT32UC3L0256

**FPGA:** Spartan 6 fpga part #: XC6SLX9-2TQG144C

## Powering PCB


## Known Issues

## TODO
* code cleanup
* function descriptions
* address warnings, currently > 100
* unit tests for functions
* in getLidar(), convert to proper units
* gpio.h includes avr32/io.h (this is missing)
* external interrupts


# UPDATES
- getLidar() is working

