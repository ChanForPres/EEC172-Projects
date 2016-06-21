//*****************************************************************************
//	Stanley Chen Kevin Fu
//	Lab 3
//*****************************************************************************

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
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#ifdef __AVR__
	#include <avr/pgmspace.h>
#endif

//**********************************************
int WIDTH,HEIGHT;
int _width  , _height  ;
int rotation;
int cursor_y ,cursor_x ; 
int textsize ;
int textcolor ,textbgcolor;
int wrap   ;
const int INT_BIT0_MIN = 51000;
const int INT_BIT0_MAX = 61000;
const int INT_BIT1_MIN = 100000;
const int INT_BIT1_MAX = 125000;
const int PERIOD =16000000;
char array[4];
char get[3];
volatile int iTick = 0;
volatile int TIME = 0;
volatile int signal1 = 0;
volatile int signal2 = 0;
volatile int COUNT = 0;
volatile int flag;
volatile char * Tx_ptr;
volatile bool Tx_done;
int rows =1;
int i=0; 
int j=0;
int k=0;
volatile int v=0;
volatile int y=64,x=62;
volatile int p=60;
int w=10; 
volatile int z=60;
int t;			//changing the paddle location
int u;			//changing the paddle location
int dx=2;			//changing the ball location 
int dy=1;
int dz=10;
#define BLACK 0x0000
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define NUM_SSI_DATA    3

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

void UART1IntHandler(void) {
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART1_BASE))
    {
			//get[v] = ROM_UARTCharGet(UART1_BASE);	//get the chars from the array 
			//v++;

		}
		//drawFillRect(124, z, 4, 25, BLACK);
		//z=get[2];
		//v=0;
		//drawFillRect(124, z, 4, 25, WHITE);
}

//*****************************************************************************
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

//*****************************************************************************
void Outstr_UART1 (char *ptr) {
	while (*ptr) {
		ROM_UARTCharPut(UART1_BASE, *ptr++);
		
	}
}
//*****************************************************************************
//*****************************************************************************
void SYS_TICK_IR_HANDLER(void)
{
	iTick++;
}
//*****************************************************************************
void IR_HANDLER(void)
{
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	TIME = PERIOD - ROM_SysTickValueGet();
	NVIC_ST_CURRENT_R = 0x00; 
	if(TIME > 500000){
		COUNT = 0;
	}
	else{
		if(COUNT < 32){
			if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX)){
				signal1 = (signal1 << 1) + 0;
			}
			else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX)
			{
				signal1 = (signal1 << 1) + 1;
			}
		}
		signal1 = signal1 & 0xFFFF;		
		
			if(signal1 == 0x807F){												// button 1 - UP
				drawFillRect(0, z, 4, 25, BLACK);
				z=z+dz;
				array[2]=z;
				drawFillRect(0, z, 4, 25, WHITE);	
				ROM_SysCtlDelay(SysCtlClockGet()/4);
				
			}
			if(signal1 == 0x20DF){											//button 4 - DOWN
				drawFillRect(0, z, 4, 25, BLACK);
				z=z-dz;
				array[2]=z;
				drawFillRect(0, z, 4, 25, WHITE);	
				ROM_SysCtlDelay(SysCtlClockGet()/4);
		
				}
//			if(signal1 == 0xC03F){											//button 3 - UP
//				drawFillRect(124, p, 4, 25, BLACK);
//				p=p+dz;
//				array[3]=p;
//				drawFillRect(124, p, 4, 25, WHITE);	
//	
//				}
//			if(signal1 == 0x609F){											//button 6 - DOWN
//				drawFillRect(124, p, 4, 25, BLACK);
//				p=p-dz;
//				array[3]=p;
//				drawFillRect(124, p, 4, 25, WHITE);	
//				}
		
		COUNT++;
		flag=0;
	
	}
	
}
//*****************************************************************************
int main(void){
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);		// Enable the GPIO port that is used for the on-board LED.
		ROM_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_3);		//Enable Pin 3 for input
		ROM_SysTickPeriodSet(16000000);			//Configuring the Interrupt Parameters to use
		ROM_SysTickEnable();
		ROM_SysTickIntEnable();
		//Interrupt is caused on the falling edges and read in from PD3
		ROM_GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
    ROM_IntEnable(INT_GPIOD);
    // Set the clocking to run directly from the external crystal/oscillator.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | 
							SYSCTL_XTAL_16MHZ);
		ROM_FPULazyStackingEnable();
    // The SSI0 peripheral must be enabled for use.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    // For this example SSI0 is used with PortA[5:2].
		// GPIO port A needs to be enabled so these pins can be used.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    // Configure the GPIO settings for the SSI pins.  This function also gives
    // control of these pins to the SSI hardware.  Consult the data sheet to
    // see which functions are allocated per pin.
    // The pins are assigned as follows:
    //      PA5 - SSI0Tx
    //      PA4 - SSI0Rx
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK
    ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_3 | GPIO_PIN_2);
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // Configure and enable the SSI port for SPI master mode.  Use SSI0,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    // For SPI mode, you can set the polarity of the SSI clock when the SSI
    // unit is idle.  You can also configure what clock edge you want to
    // capture data on.  Please reference the datasheet for more information on
    // the different SPI modes.
    ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);
		// Enable interrupts on UART1 for receive
		ROM_IntEnable(INT_UART1);
    ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
		ROM_UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);
		ROM_IntMasterEnable();
    // Enable the SSI0 module.
    ConfigureUART1();
		ROM_SSIEnable(SSI0_BASE);
		reset();
		Adafruit_SSD1351_Init(128,128);
		begin();
		InitConsole();
		fillScreen(BLACK);										//Fills the background screen black
		drawFillRect(0, 60, 4, 25, WHITE);		//Possibly a long LEFT paddle
		drawFillRect(124, 60, 4, 25, WHITE);		//Possibly a long RIGHT paddle OR Vice Versa
		fillCircle(64,64,4,BLUE);							//Cirlce that will start in the middle of screen
		while(1){
			ROM_SysCtlSleep();
			while(1){
				while (x!=125 && y!= 125){
					fillCircle(x,y,4,BLACK);
					if (x>=124)
						dx=dx*-1;    //Circle will move right (+) and left(-)
					if(y>=124)
						dy=dy*-1;
					if(x<=3)
						dx=dx*-1;
					if(y<=3)
						dy=dy*-1;
					x=x+dx;
					y=y+dy;
					array[0]=x;
					array[1]=y;
					Outstr_UART1(array);		//coordinate for the ball
					fillCircle(x,y,4,BLUE);	
					ROM_SysCtlDelay(SysCtlClockGet()*2/300);		//makes the ball slower
					k=0;
				}
			COUNT=0;
			}
			
			
			
			
			
		// need to make the count or iTick % by 400 or something to give the signal some time to process correctly
		// Update the ball and paddle in the while loop 
		}
    return(0);
}
//*****************************************************************************
