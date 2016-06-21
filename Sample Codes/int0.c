// Int0.c
// Example program to demonstrate switch interrupts
// Note that PF0 must be unlocked so it can be used as a GPIO. (special case)
// Generate interrupt when either SW1 or SW2 pressed and toggle LED.
// Disable interrupts after interrupt to avoid multiple interrupts due to switch bounce


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

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

void Switch_Handler (void) {
	int leds;

	// Disable interrupt for awhile to avoid switch bounce
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);

	// Clear interrupt request
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
	
	// Toggle LEDs	
	leds = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, ~leds);
}	


//*****************************************************************************
int
main(void)
{
		// Set system clock to 50 MHz	
	  ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);		

		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	// Enable the GPIO port F

		//
    // Unlock PF0 so we can change it to a GPIO input
    // Once we have enabled (unlocked) the commit register then re-lock it
    // to prevent further changes.  PF0 is muxed with NMI thus a special case.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

		// Configure PF0 and PF4 as inputs to read switch values
		// Configure as inputs before configuring pad with pull-up resistors
		ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);	// PF0, PF4 = switches

		// Need pull-up resistors on switch inputs
		// Call this after pins configured as inputs
		ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

		// Configure PF1 (Red LED), PF2 (Blue LED) and PF3 (Green LED) as outputs
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

		ROM_GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_FALLING_EDGE);

		GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
		
		ROM_IntEnable(INT_GPIOF);
		
		ROM_IntMasterEnable();
		
		// Turn on Green LED only		
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_3);
	
    while(1)
    {
			// Low power sleep mode while waiting for interrupt			
			ROM_SysCtlSleep();
			
			// After interrupt, delay during switch bounce		
			ROM_SysCtlDelay(SysCtlClockGet() / 4 / 3);

			// Clear pending interrupt (switch bounce) and re-enable interrupts			
			GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
			GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
    }
}
