//*****************************************************************************
//
// spi_master.c - Example demonstrating how to configure SSI0 in SPI master
//                mode.
//
// Copyright (c) 2010-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
// 
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the  
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This is part of revision 2.1.0.12573 of the Tiva Firmware Development Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"

#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
//#include "ino.h"
//**********************************************
int WIDTH,HEIGHT;
int _width  , _height  ;
int rotation;
int cursor_y ,cursor_x ; 
int textsize ;
int textcolor ,textbgcolor;
int wrap   ;

//********************************************
//*****************************************************************************
//
//! \addtogroup ssi_examples_list
//! <h1>SPI Master (spi_master)</h1>
//!
//! This example shows how to configure the SSI0 as SPI Master.  The code will
//! send three characters on the master Tx then polls the receive FIFO until
//! 3 characters are received on the master Rx.
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SSI0 peripheral
//! - GPIO Port A peripheral (for SSI0 pins)
//! - SSI0Clk - PA2
//! - SSI0Fss - PA3
//! - SSI0Rx  - PA4
//! - SSI0Tx  - PA5
//!
//! The following UART signals are configured only for displaying console
//! messages for this example.  These are not required for operation of SSI0.
//! - UART0 peripheral
//! - GPIO Port A peripheral (for UART0 pins)
//! - UART0RX - PA0
//! - UART0TX - PA1
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - None.
//
//*****************************************************************************

//*****************************************************************************
//
// Number of bytes to send and receive.
//
//*****************************************************************************
#define NUM_SSI_DATA            3

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************


void
InitConsole(void)
{
    // Enable GPIO port A which is used for UART0 pins.
    // Enable UART0.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	//  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // Select the alternate (UART) function for these pins.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    // Initialize UART0 for console I/O. (See uartstdio.c)
    //
		UARTStdioConfig(0, 115200, 16000000);


// configure SSI 

}


//*****************************************************************************
void horizontal8Bands(){
 int i;
	
	fillScreen(0x0000);
  for(i=0;i<128;i+=5)
		drawFastHLine(0,i,128,0xFFFF);
	delay(500);
}
	
	void vertical8Bands(){
		int i;
	
		fillScreen(0x0000);
		
	for(i=0;i<128;i+=5)
		drawFastVLine(i,0,128,0xFFFF);
		delay(500);
	}
	
void printHelloWorld(){
	int col=10;
	fillScreen(0x0000);
	
	drawChar(1,1,'H',0xFFFF,0x000F,2);
	drawChar(col,1,'e',0xFFFF,0x000F,2);
	drawChar(col*2,1,'l',0xFFFF,0x000F,2);
	drawChar(col*3,1,'l',0xFFFF,0x000F,2);
	drawChar(col*4,1,'o',0xFFFF,0x000F,2);
	//drawChar(col*5,1,' ',0xFFFF,0x000F,1);
	drawChar(col*6,1,'w',0xFFFF,0x000F,2);
	drawChar(col*7,1,'o',0xFFFF,0x000F,2);
	drawChar(col*8,1,'r',0xFFFF,0x000F,2);
	drawChar(col*9,1,'l',0xFFFF,0x000F,2);
	drawChar(col*10,1,'d',0xFFFF,0x000F,2);
	drawChar(col*11,1,'!',0xFFFF,0x000F,2);
	delay(500);
}
void printFont(){
//line = pgm_read_byte(font+(c*5)+i);
	int16_t i,j;
	int16_t y=0,x=0;
	int16_t w=10;
	i=0;
	j=0;
	fillScreen(0x0000);
	while(i<255){
		drawChar(x++*w,y,(unsigned char)i++,0xFFFF,0x000F,2);
		
		if(i%12==0) { 
			y +=18;
			x=0;
			delay(1000);
		}
		
		if(i%96==0) {
		delay(1000);
			fillScreen(0x0000);
			x=0;
			y=0;
		}
	}
}

//*****************************************************************************
//
// Configure SSI0 in master Freescale (SPI) mode.  This example will send out
// 3 bytes of data, then wait for 3 bytes of data to come in.  This will all be
// done using the polling method.
//

int main(void) {
	/*
    uint32_t pui32DataTx[NUM_SSI_DATA];
    uint32_t pui32DataRx[NUM_SSI_DATA];
    uint32_t ui32Index;
	*/
	

    //
    // Set the clocking to run directly from the external crystal/oscillator.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | 
							SYSCTL_XTAL_16MHZ);

    // The SSI0 peripheral must be enabled for use.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // For this example SSI0 is used with PortA[5:2].
		// GPIO port A needs to be enabled so these pins can be used.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    //
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    //GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    //
    // Configure the GPIO settings for the SSI pins.  This function also gives
    // control of these pins to the SSI hardware.  Consult the data sheet to
    // see which functions are allocated per pin.
    // The pins are assigned as follows:
    //      PA5 - SSI0Tx
    //      PA4 - SSI0Rx
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK
    //
    ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_3 |
                   GPIO_PIN_2);
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    //
    // Configure and enable the SSI port for SPI master mode.  Use SSI0,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    // For SPI mode, you can set the polarity of the SSI clock when the SSI
    // unit is idle.  You can also configure what clock edge you want to
    // capture data on.  Please reference the datasheet for more information on
    // the different SPI modes.
    //
    ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);

    //
    // Enable the SSI0 module.
    //
    ROM_SSIEnable(SSI0_BASE);
	
	reset();
  Adafruit_SSD1351_Init(128,128);
	 begin();
	 fillScreen(0x00);
	 
		while(1){
printFont();
			delay(20);
printHelloWorld();
			delay(20);
horizontal8Bands();
			delay(20);
vertical8Bands();			
			delay(20);
testlines(0xFFFF) ;
			delay(20); //delay 20us
testfastlines(0xFFFF, 0x000A) ;
			delay(20); //delay 20us
testdrawrects(0xFFFF) ;
			delay(20); //delay 20us
testfillrects(0xFFFF, 0x000A) ;
			delay(20); //delay 20us
testfillcircles(10, 0x000A) ;
			delay(20); //delay 20us
testdrawcircles(5, 0xFFFA) ;
			delay(20); //delay 20us
testtriangles() ;
			delay(20); //delay 20us
testroundrects();	
			delay(20); //delay 20us
		}
		
		
		
    return(0);
}

/*
void testFillCircle(){

}
void testDrawCircles(){
	drawCircle(64,64,30,0xFFF0);
}
void testFillRect(){
	drawRect(1,1,120,120,0xFFFF);
	drawFillRect(2,2,119,119,0x00AF);
}
void testFastLine(){
	
	int i;
	drawRect(0,0,128,128,0xFFFF);
	for(i=1;i<128;i+=5){
		drawFastHLine(0,i,127,0x00F5);
		drawFastVLine(i,0,127,0x00F5);
	}

}
void testDrawRect(){
int i;
	for (i=0; i<128;i+=5){
		drawRect(i,i,128-(2*i),128-(2*i),0xFFF0);
	}
}


void testRoundRect(){}
void testTriangle(){}

	void testLine(){
		drawLine(0,0,128,128,0xFFF0);
	//	drawLine(128,0,0,128,0xFF00);
	}
*/
