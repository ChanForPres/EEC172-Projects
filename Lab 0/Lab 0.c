/*

	IR Receiver Program
	Stanley Chen
    EEC 172
    996735641

*/

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
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

/*
	Creates the global constants
		- Range of time for 0 and 1 bit 
		- Period is used as a base to check how long it took to count down
		  to the next falling edge
		- Time: Time between two falling edges
		- signal: stores the 32 bits that are received
		- COUNT: keeps count of the amount of 0 and 1 that are being received
		  will let us know what we should use as useful information in the case
		  that the button is held down
*/
const int INT_BIT0_MIN = 50000;
const int INT_BIT0_MAX = 60000;
const int INT_BIT1_MIN = 100000;
const int INT_BIT1_MAX = 120000;
const int PERIOD =16000000;

volatile int iTick = 0;
volatile int TIME = 0;
volatile int signal = 0;
volatile int COUNT = 0;
bool CLEAR_FLAG = true;


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
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}


void SYS_TICK_IR_HANDLER(void)
{
	iTick++;
}

/*
	This is my interrupt handler. Whenever an interrupt is detected on the falling edge,
	it will call this function. I use PD3 to detect the time difference. The time difference
	between falling edges determines what value was sent. 
*/
void IR_HANDLER(void)
{
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	TIME = PERIOD - ROM_SysTickValueGet();
	NVIC_ST_CURRENT_R = 0x00; 
	
	/*
		Make sure that the Time difference isn't too long; otherwise it is the beginning
		If it isn't too long, then we can start taking down the first 32 time measurements
		the signal's time is checked between the ranges and determined to be a 0 or 1
		Two possible ways of storing: array or int. I chose int because this means I wouldn't
		have to deal with locations. In the int, all I have to do is shift it left once and then
		add the new bit digit.
	*/
	if(TIME > 500000)
	{
		COUNT = 0;
		signal = 0;
		CLEAR_FLAG = false;
	}
	else
	{
		CLEAR_FLAG = true;
	}
	if(COUNT <= 32 && CLEAR_FLAG == true)
	{
		if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX))
		{
			signal = (signal << 1) + 0;
		}
		else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX)
		{
			signal = (signal << 1) + 1;
		}
	}
	/*
		Signal is comprised of 32 bits. The first 16 bits are the address which is the same
		due to the address corresponding to a certain brand/machine. Hence, this isn't important because
		we are assuming that the receiver is compatible with the transmission type. The last 16 bits will always
		differ due it corresponding to the button that is being pressed. The 32 bits are then ANDED with 0x0000FFFFF
		which will leave us with the last 16 bits. Then, those 16 bits are compared to 
		the hexadecimal values of the button. I chose to use if statements (could use switches too).
	*/
	if(COUNT == 32)
	{
		signal = signal & 0xFFFF;
		if(signal == 0x807F)
			UARTprintf("1\n");
		else if(signal == 0x40BF)
			UARTprintf("2\n");
		else if(signal == 0xC03F)
			UARTprintf("3\n");
		else if(signal == 0x20DF)
			UARTprintf("4\n");
		else if(signal == 0xA05F)
			UARTprintf("5\n");
		else if(signal == 0x609F)
			UARTprintf("6\n");
		else if(signal == 0xE01F)
			UARTprintf("7\n");
		else if(signal == 0x10EF)
			UARTprintf("8\n");
		else if(signal == 0x906F)
			UARTprintf("9\n");
		else if(signal == 0x00FF)
			UARTprintf("0\n");
		else if(signal == 0x6E91)
			UARTprintf("<\n");
		else if(signal == 0xAE51)
			UARTprintf(">\n");
		else if(signal == 0xCE31)
			UARTprintf("^\n");
		else if(signal == 0x2ED1)
			UARTprintf("V\n");
		else if(signal == 0x26D9)
			UARTprintf("MUTE\n");
		
	}
    /*
        Count keeps counting but we only care about the first 32 bits that are sent.
        If it is held down, then it will continously send the same signal for the duration
        that it is held down. When it is let go however, it will create a large period of time
        where TIME will be extremely large (which is how we will detect a button be let go).
    */
	COUNT++;
}

void initialize(void)
{
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    ROM_FPULazyStackingEnable();
	
    // Set the clocking to run directly from the crystal.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Enable the GPIO port that is used for the on-board LED.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	
	//Enable Pin 3 for input
	ROM_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_3);
	
	/*
		Configuring the Interrupt Parameters to use
	*/
	ROM_SysTickPeriodSet(16000000);
	ROM_SysTickEnable();
	ROM_SysTickIntEnable();
	
	ROM_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_3);
	/*
		Interrupt is caused on the falling edges and read in from PD3
	*/
	ROM_GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
    ROM_IntEnable(INT_GPIOD);
    ROM_IntMasterEnable();
		
	//Configure UART
	ConfigureUART();
	
}
int main(void)
{
	
	initialize();
	while(1)
	{
		ROM_SysCtlSleep();
		
			
	}
	
}
