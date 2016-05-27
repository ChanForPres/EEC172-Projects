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

volatile int iTick = 0;
volatile int TIME = 0;
volatile int signal1 = 0;
volatile int COUNT = 0;
volatile int i=0;
volatile int y=0,x=0;
volatile int w=10;
volatile int character = 0;
volatile int loc = 0;

// Lab 2 Part 3
char message[50];
volatile int len_msg = 0;

// Lab 3 Part 3
volatile int y1 = 0, y2 = 0;
volatile int x1 = 0, x2 = 0;
volatile int xpos = 64, ypos = 64;
int xmitLoc[4];

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
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);

    // Initialize UART1
    //
		ROM_UARTConfigSetExpClk(UART1_BASE, 16000000, 9600,
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

void UART1IntHandler(void) {
    uint32_t ui32Status;
		int temp[4];

    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART1_BASE, ui32Status);

    while(ROM_UARTCharsAvail(UART1_BASE))
    {
				temp[loc] = ROM_UARTCharGet(UART1_BASE);
				loc++;
    }
		
		//reset loc to be used the next iteration
		loc = 0;
		drawRect(x2,y2, 6, 20, BLACK);
		y2 = temp[0];
		drawRect(x2,y2, 6, 20, WHITE);
		
		drawRect(x1,y1, 6, 20, BLACK);
		y1 = temp[1];
		drawRect(x1,y1, 6, 20, BLUE);
	
		fillCircle(xpos,ypos,4,BLACK);
		xpos = temp[2];
		ypos = temp[3];
		fillCircle(xpos, ypos, 4, WHITE);

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
	ROM_IntMasterEnable();
}

void SYS_TICK_IR_HANDLER(void)
{
	iTick++;
}

void IR_HANDLER(void)
{
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	TIME = PERIOD - ROM_SysTickValueGet();
	NVIC_ST_CURRENT_R = 0x00; 
	UARTprintf("%d\n",TIME);
	if(TIME > 500000){
		COUNT = 0;
		signal1 =0;
	}
	else
	{
		if(COUNT <= 32)
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
	}
	COUNT++;
	if(COUNT == 33)
	{
		signal1 = signal1 & 0xFFFF;
		if(signal1 == 0x06F9)
		{
			if(y2+10 > 105)
			{
			}
			else
			{
				drawRect(x2,y2, 6, 20, BLACK);
				y2 = y2+10;
				drawRect(x2,y2, 6, 20, WHITE);
			}
		}
		else if (signal1 == 0x8679)
		{
			if(y2-10 < 0)
			{
			}
			else
			{
				drawRect(x2,y2, 6, 20, BLACK);
				y2 = y2-10;
				drawRect(x2,y2, 6, 20, WHITE);
			}
		}
		else if (signal1 == 0xB04F)
		{
			if(y1-10 < 0)
			{
			}
			else
			{
				drawRect(x1,y1, 6, 20, BLACK);
				y1 = y1-10;
				drawRect(x1,y1, 6, 20, BLUE);
			}
		}
		else if (signal1 == 0x30CF)
		{
			if(y1+10 > 105)
			{
			}
			else
			{
				drawRect(x1,y1, 6, 20, BLACK);
				y1 = y1+10;
				drawRect(x1,y1, 6, 20, BLUE);
			}
		}
		}
		xmitLoc[0] = y2;
		xmitLoc[1] = y1;
	}


int main(void){
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

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
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    //
    ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_3 |
                   GPIO_PIN_2);
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);

    // Enable the SSI0 module.
    //
		ROM_SSIEnable(SSI0_BASE);

		InitConsole();
		initialize();
		reset();
		Adafruit_SSD1351_Init(128,128);
		begin();
		ConfigureUART1();
		ROM_IntEnable(INT_UART1);
		ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
		ROM_IntEnable(INT_UART0);
		ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
		
		// Starts off with straight line 
		fillScreen(BLACK);
		int dx = 4;
		int dy = 0;
		y1 = 44;
		y2 = 84;
		x1 = 122;
		x2 = 5;
		fillScreen(BLACK);
		
		// Initialize the beginning of hte game
		drawRect(x1,y1, 6, 20, BLUE);
		drawRect(x2,y2, 6, 20, WHITE);
		drawCircle(xpos, ypos, 4, WHITE);
		
		//Play the game
		while(1){
			Outstr_UART1((uint8_t *)xmitLoc);
			ROM_SysCtlDelay(SysCtlClockGet()/10);
			
			fillCircle(xpos,ypos,4,BLACK); 	
			
			if (xpos >= 122 || xpos <= 0)
			{
				fillScreen(BLACK);
				break;
			}
			// Blue Paddle Logic
			if (xpos >= 115
				&& ypos >= y1 && ypos <= y1+20)
			{
				if(ypos <= y1+6 ){
					dx = dx*-1;
					dy = -1;
				}
				else if ( (ypos > y1+6) && (ypos < y1+14))
				{
					dx = dx*-1;
				}
				else
				{
					dx = dx*-1;
					dy = 1;
				}
			}
			
			// White Paddle Logic
			if(xpos <= 20 && ypos >= y2 && ypos <= y2+20)
			{
				if(ypos <= y2+6)
				{
					dx = dx*-1;
					dy = -1;
				}
				else if( (ypos > y2+6) && (ypos < y2+14))
				{
					dx = dx*-1;
				}
				else
				{
					dx = dx*-1;
					dy = 1;
				}
			}
			// if it hits the top make it bounce the other way
			if (ypos >= 127 || ypos <= 0)
			{
				dy = dy*-1;
			}
			//update ball
			xpos = xpos + dx;
			ypos = ypos + dy;
			fillCircle(xpos, ypos, 4, WHITE);
			drawRect(x2,y2, 6, 20, WHITE);
			drawRect(x1,y1, 6, 20, BLUE);
			// update all values for transmission
			xmitLoc[0] = y2;
			xmitLoc[1] = y1;
			
		}
    return(0);
}
