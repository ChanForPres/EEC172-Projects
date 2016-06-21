// Stanley Chen & Kevin Fu
// 996735641 998279066

#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"

#include "driverlib/Adafruit_SSD1351.h"
#include "glcdfont.h"

//**********************************************
int WIDTH,HEIGHT;
int _width  , _height  ;
int rotation;
int cursor_y ,cursor_x ; 
int textsize ;
int textcolor ,textbgcolor;
int wrap   ;
const int INT_BIT0_MIN = 50000;
const int INT_BIT0_MAX = 60000;
const int INT_BIT1_MIN = 100000;
const int INT_BIT1_MAX = 120000;
const int PERIOD =16000000;

volatile int repeat = 0;
volatile int iTick = 0;
volatile int TIME = 0;
volatile int signal1 = 0;
volatile int signal2 = 0;
volatile int COUNT = 0;
volatile int counter = 0;
volatile int i=0;
volatile int y=0,x=0;
volatile int w=10;
volatile int character = 0;
volatile int arrayloc = 0;

// Lab 2 Part 3
char message[50];
volatile int len_msg = 0;

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
	
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    // Initialize UART0 for console I/O. (See uartstdio.c)
		//UARTStdioConfig(0, 9, 16000000);
}

void ConfigureUART1(void) {
	
		// Enable the GPIO Peripheral used by the UART1 and enable UART1
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    //
    //UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);

    // Initialize UART1
    //
		ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet() , 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
}

void Outstr_UART1 (uint8_t *ptr) {

	while (*ptr) {
		ROM_UARTCharPut(UART1_BASE, *ptr++);
	}
}

void UART0IntHandler(void) {
    uint32_t ui32Status;
		uint8_t in_char, out_char;
	
    //
    // Get the interrrupt status.
    //
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART0_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(ROM_UARTCharsAvail(UART0_BASE))
    {
        //
        // Read the next character from the UART0 echo it back.
				// Convert to uppercase and sent to remote processor
				// We can use NonBlocking Get since we know character is available.
        //
				in_char = ROM_UARTCharGet(UART0_BASE);
        ROM_UARTCharPut(UART0_BASE, in_char);

				if ((in_char >=  'a') && (in_char <= 'z')) {
					out_char = in_char - ('a' - 'A');		// Capatilize lower case letters
				} else {
					out_char = in_char;
				}

        ROM_UARTCharPut(UART1_BASE, out_char);
    }	
}


//*****************************************************************************
// Receives character from other processor and displays on console window.

void UART1IntHandler(void) {
    uint32_t ui32Status;
		uint8_t leds;
		int x = 0;
		int rows = 0;

    //
    // Get the interrrupt status.
    //
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART1_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(ROM_UARTCharsAvail(UART1_BASE))
    {
					character = ROM_UARTCharGet(UART1_BASE);
			    drawChar(x*8, 9*8, character, YELLOW, BLACK, 1);
					x++;
					if(((x+1) % 16) == 0) 
					{
							rows++;
							x = 0;
					}
    }
}

/*
	Translate function is called whenever a signal needs to be outputted to the 
	OLED screen. It happens when signal 1 and signal 2 are not equal to one another.
	It mods repeat by either 3 or 4 and depending on the output it'll correspond to
	the location of the letter under that number. 
	(ex. 1 is pressed / repeat is 2 then the output is C)

	Also, it stores to the message array, which will be fed to the FIFO and out for 
	transfer when the enter key is pressed.

	If enter is pressed, it resets EVERYTHING, because we want to start fresh again.
*/

void translate(void)
{
			int value = 0;
			value = repeat % 3;
			signal1 = signal1 & 0xFFFF;
			if(signal1 == 0x40BF)
			{
					drawChar(x*w,y,'A'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'A'+value;
					arrayloc++;
			}
			else if(signal1 == 0xC03F)
			{
					drawChar(x*w,y,'D'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'D'+value;
					arrayloc++;
			}
			else if(signal1 == 0x20DF)
			{
					drawChar(x*w,y,'G'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'G'+value;
					arrayloc++;
			}
			else if(signal1 == 0xA05F)
			{
					drawChar(x*w,y,'J'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'J'+value;
					arrayloc++;
			}
			else if(signal1 == 0x609F)
			{
					drawChar(x*w,y,'M'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'M'+value;
					arrayloc++;
			}
			else if(signal1 == 0xE01F)
			{
				value = repeat % 4;
				drawChar(x*w,y,'P'+value,0xFFFF,0x000F,1);
				message[arrayloc] = 'P'+value;
				arrayloc++;
			}
			else if(signal1 == 0x10EF)
			{
					drawChar(x*w,y,'T'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'T'+value;
					arrayloc++;
			}
			else if(signal1 == 0x906F)
			{
					value = repeat % 4;
					drawChar(x*w,y,'W'+value,0xFFFF,0x000F,1);
					message[arrayloc] = 'W'+value;
					arrayloc++;
			}
			else if(signal1 == 0x00FF)
			{
				x++;
				message[arrayloc] = ' ';
				arrayloc++;
			}
			if(signal1 == 0x26D9)
			{
					Outstr_UART1((uint8_t *)message);
					// Resets everything
					signal1 = 0;
					COUNT = 0;
					signal2 = 0;
					repeat = 0;
					arrayloc = 0;
			}
			
			if(signal2 != 0)
			{
				i++;
				x++;
				if(i%12==0) { 
					y +=18;
					x=0;
					delay(1000);
				}
			}
}

void initialize(void)
{

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
	
	/*
		Interrupt is caused on the falling edges and read in from PD3
	*/
	ROM_GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	ROM_IntEnable(INT_GPIOD);

}

void SYS_TICK_IR_HANDLER(void)
{
	iTick++;
}

/*
	This is my interrupt handler. Whenever an interrupt is detected on the falling edge,
	it will call this function. I use PD3 to detect the time difference. The time difference
	between falling edges determines what value was sent. 

	if TIME is too big then we know it just started and we don't take it into consideration
	I have two signals: signal 1 and signal 2. Signal 1 holds the 'current' signal and signal 2
	holds the next one that comes in.
				- If signal 1 and signal 2 are equal then increment 
					the repeat variable (keeps count how many times its been clicked) 
					and set signal 2 to 0
				- If they're different, then print signal 1 and then set signal1 to signal 2
					this way it holds onto the next one to keep count of in case there are repeats
				- if an interrupt is triggered in here counter will be set to 0 to make sure the clock
					essentially resets

*/
void IR_HANDLER(void)
{
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	TIME = PERIOD - ROM_SysTickValueGet();
	NVIC_ST_CURRENT_R = 0x00; 
	if(TIME > 500000){}
	else
	{
		if(COUNT < 32)
		{
			if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX))
			{
				signal1 = (signal1 << 1) + 0;
			}
			else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX)
			{
				signal1 = (signal1 << 1) + 1;
			}
		}
		else
		{
			if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX))
			{
				signal2 = (signal2 << 1) + 0;
			}
			else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX)
			{
				signal2 = (signal2 << 1) + 1;
			}
			if((COUNT+1) % 32 == 0)
			{
				if( signal1 == signal2 )
				{
					repeat++;
					signal2 = 0;
				}
				else
				{
					translate();
					repeat = 0;
					signal1 = signal2;
					signal2 = 0;
					
				}
			}
		}
		COUNT++;
		counter = 0;
	}
}

/*
	Logic:
		- Configure all clock, Pins for Output/SSI/Input/UART
		- Run a while loop
				- Sleep and wait for interrupt
				- Poll for 2 seconds and check if there is an interrupt
								- If interrupt is hit, it resets the clock/count to zero (resetting clock)
				- If it finishes polling for 2 seconds,
								- Print current character in cycle and set the signals and repeat to 0
				- Run always 
*/
int main(void){
	
    // Set the clocking to run directly from the external crystal/oscillator.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | 
							SYSCTL_XTAL_16MHZ);

    // The SSI0 peripheral must be enabled for use.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // For this example SSI0 is used with PortA[5:2].
		// GPIO port A needs to be enabled so these pins can be used.
    
		InitConsole();
		ConfigureUART1();

    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    //
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
	
    // The pins are assigned as follows:
    //      PA5 - SSI0Tx
    //      PA4 - SSI0Rx
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK
    //
    ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_3 |
                   GPIO_PIN_2);
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);

    // Enable the SSI0 module.
    //
		ROM_SSIEnable(SSI0_BASE);

		initialize();
		reset();
		Adafruit_SSD1351_Init(128,128);
		begin();
		ROM_IntEnable(INT_UART1);
		ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
		UARTprintf("hi.");
		ROM_IntMasterEnable();
		fillScreen(BLACK);
		drawFastHLine(0, 64, 128, YELLOW);
		while(1){
			//ROM_SysCtlSleep();
			for(counter = 0; counter < 100; counter ++)
			{
				ROM_SysCtlDelay(SysCtlClockGet()*2/300);
			}
			if(counter == 100)
			{
				signal2 = 0;
				translate();
				signal1 = 0;
				repeat = 0;
			}
		}
		
    return(0);
}
