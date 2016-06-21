// SysTick0.c
// Example program to demonstrate SysTick interrupts
// Generate SysTick interrupts at 1 kHz
// Toggle LEDs every 1 second


#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_nvic.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

volatile int iTick=0;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************

void SysTick_Handler (void) {
	int leds;
	
	iTick++;
	if (iTick == 1000) {
		leds = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, ~leds);
		iTick = 0;
	}
}	


//*****************************************************************************
int
main(void)
{
	
	// Set system clock to 50 MHz	
	  ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);		

		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	// Enable the GPIO port F

		// Configure PF1 (Red LED), PF2 (Blue LED) and PF3 (Green LED) as outputs
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

/*	
//	This code uses TivaWare to set up 1 kHz interrupts using system clock
//	Comment out when using register-direct code to configure SysTick
	
		ROM_SysTickPeriodSet (SysCtlClockGet()/1000);	
		ROM_SysTickIntEnable();
		ROM_SysTickEnable();
*/

//	This code uses PIOSC/4 as the clock source and generates 1 kHz interrupts	 
//	Comment out when using TivaWare functions to configure SysTick
	
		NVIC_ST_CTRL_R = 0x00;		// clear CLK_SRC bit so PIOSC/4 is SysTick's clock source
		NVIC_ST_RELOAD_R = 4000; 	// (16MHz/4) / 1000 = 4000
		NVIC_ST_CTRL_R = 0x03;		// enable interrupts and enable timer

		NVIC_ST_CURRENT_R = 0x00;	// clear SysTick counter (not necessary - just as example)

		// Turn on Green LED only		
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_3);
	
    while(1)
    {
			// Low power sleep mode while waiting for interrupt			
			ROM_SysCtlSleep();			
		}
}
