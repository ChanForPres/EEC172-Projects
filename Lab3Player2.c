//*****************************************************************************
//	Elizabeth Ear and Jason Woo
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


char MessArray[50];
char get[20];
volatile int repeat = 0;
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
int y=64,x=62;
int w=10;
int t;			//changing the paddle location
int u;			//changing the paddle location
int dx=2;			//changing the ball location 
int dy=1;
int dx2=3;
int dy2=4;
#define BLACK 0x0000
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

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
		UARTprintf("Init");
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
		int z;
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART1_BASE))
    {
			get[z] = ROM_UARTCharGet(UART1_BASE);	//get the chars from the array 
		}
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
void Outstr_UART1 (uint8_t *ptr) {
	while (*ptr) {
		ROM_UARTCharPut(UART1_BASE, *ptr++);
	}
}
//*****************************************************************************
void translate(void){
			int value = 0;
	
	///might not even need this part so comment out 
			value = repeat % 3;								//determines the specific char from button 
			signal1 = signal1 & 0xFFFF;				
			if(signal1 == 0x08F7){						//mute key
				drawChar(x++*w,y,'+',WHITE,0x000F,1);
				Outstr_UART1((uint8_t *)MessArray); //cast it because it took in the parameters as ints
				signal1=0;
				COUNT=0;
				signal2=0;
				repeat=0;
			}
			else if(signal1 == 0x40BF){											//button 2
				if (value == 0) {
					drawChar(x++*w,y,'A',WHITE,0x000F,1);
					MessArray[k]='A';
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
					k++;
					}
				else if (value == 1){
					drawChar(x++*w,y,'B',WHITE,0x000F,1);
					MessArray[k]='B';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
					}
				else{
					drawChar(x++*w,y,'C',WHITE,0x000F,1);
					MessArray[k]='C';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
					}
					i++;
				}
				}
			else if(signal1 == 0xC03F){											//button 3
				if (value == 0) {
					drawChar(x++*w,y,'D',WHITE,0x000F,1);
					MessArray[k]='D';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				
				}
				else if (value == 1){
					drawChar(x++*w,y,'E',WHITE,0x000F,1);
					MessArray[k]='E';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				
				}
				else{
					drawChar(x++*w,y,'F',WHITE,0x000F,1);
					MessArray[k]='F';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0x20DF){													//button 4
				if (value == 0) {
					drawChar(x++*w,y,'G',WHITE,0x000F,1);
					MessArray[k]='G';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 1){
					drawChar(x++*w,y,'H',WHITE,0x000F,1);
					MessArray[k]='H';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else{
					drawChar(x++*w,y,'I',WHITE,0x000F,1);
					MessArray[k]='I';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0xA05F){														//button 5
				if (value == 0) {
					drawChar(x++*w,y,'J',WHITE,0x000F,1);
					MessArray[k]='J';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 1){
					drawChar(x++*w,y,'K',WHITE,0x000F,1);
					MessArray[k]='K';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else{
					drawChar(x++*w,y,'L',WHITE,0x000F,1);
					MessArray[k]='L';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0x609F){													//button 6
				if (value == 0) {
					drawChar(x++*w,y,'M',WHITE,0x000F,1);
					MessArray[k]='M';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 1){
					drawChar(x++*w,y,'N',WHITE,0x000F,1);
					MessArray[k]='N';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else{
					drawChar(x++*w,y,'O',WHITE,0x000F,1);
					MessArray[k]='O';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0xE01F){														//button 7 
				value = repeat % 4;
				if (value == 0){
					drawChar(x++*w,y,'P',WHITE,0x000F,1);
					MessArray[k]='P';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 1){
					drawChar(x++*w,y,'Q',WHITE,0x000F,1);
					MessArray[k]='Q';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 3){
					drawChar(x++*w,y,'R',WHITE,0x000F,1);
					MessArray[k]='R';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else{
					drawChar(x++*w,y,'S',WHITE,0x000F,1);
					MessArray[k]='S';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0x10EF){												//button 8
				if (value == 0) {
					drawChar(x++*w,y,'T',WHITE,0x000F,1);
					MessArray[k]='T';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 1){
					drawChar(x++*w,y,'U',WHITE,0x000F,1);
					MessArray[k]='U';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else{
					drawChar(x++*w,y,'V',WHITE,0x000F,1);
					MessArray[k]='V';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0x906F){												//button 9
				value = repeat % 4;
				if (value == 0) {
					drawChar(x++*w,y,'W',WHITE,0x000F,1);
					MessArray[k]='W';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 1){
					drawChar(x++*w,y,'X',WHITE,0x000F,1);
					MessArray[k]='X';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else if (value == 2){
					drawChar(x++*w,y,'Y',WHITE,0x000F,1);
					MessArray[k]='Y';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
				else{
					drawChar(x++*w,y,'Z',WHITE,0x000F,1);
					MessArray[k]='Z';
					k++;
					if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
				}
			}
			else if(signal1 == 0x00FF){			//button 0 
				drawChar(x++*w,y,' ',WHITE,0x000F,1);
					MessArray[k]=' ';
					k++;
				if(i%12==0 && i!=0) { 
						y = y + 18;
						x=0;
						}
					i++;
			}
}
//*****************************************************************************
void SIGNAL(void){
	if(TIME > 500000)																						// grabbing the signal
	{COUNT = COUNT;}
	else{
		if(COUNT < 32){																					// when signal is finally grabbed
			if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX))			// makes signal into a 0 bit
			{ signal1 = (signal1 << 1) + 0;}
			else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX)			// makes signal into a 1 bit
			{signal1 = (signal1 << 1) + 1;}
		}
		else{
			if((COUNT+1) % 32 == 0){											// if the count is a mulitple of 32 and not 32
				if(signal1 == signal2){																// same button pressed
					repeat++;
					signal2 = 0;
				}
				else{																									// different button pressed
					translate();																				// decodes the signal
					repeat = 0;																					// resets the repeat bc its a new character 
					signal1 = signal2;																	// resetting 2nd signal
					signal2 = 0;																				
				}
			}
			if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX))			// second signal 
			{signal2 = (signal2 << 1) + 0;}													// makes signal into a 0 bit
			else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX)
			{signal2 = (signal2 << 1) + 1;}													// makes signal into a 1 bit
		}
		COUNT++;
		flag=0;
	}
}
//*****************************************************************************
void SYS_TICK_IR_HANDLER(void)
{
	iTick++;
}
//*****************************************************************************
void IR_HANDLER(void)
{
	UARTprintf("IR");
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	TIME = PERIOD - ROM_SysTickValueGet();
	NVIC_ST_CURRENT_R = 0x00; 
	if(TIME > 500000){
		COUNT = COUNT;
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
		else{
			if((TIME > INT_BIT0_MIN) && (TIME < INT_BIT0_MAX)){
				signal2 = (signal2 << 1) + 0;
			}
			else if(TIME > INT_BIT1_MIN && TIME < INT_BIT1_MAX){
				signal2 = (signal2 << 1) + 1;
			}
			if((COUNT+1) % 32 == 0){
				if( signal1 == signal2 ){
					repeat++;
					signal2 = 0;
				}
				else{
					translate();
					repeat = 0;
					signal1 = signal2;
					signal2 = 0;
				}
			}
		}
		COUNT++;
		flag=0;
	}
}
//*****************************************************************************
int main(void){
	UARTprintf("MAIN");
		
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
			ConfigureUART1();
			
			for (flag=0;flag<100;flag++){
			ROM_SysCtlDelay(SysCtlClockGet()*2/300);}
				
			if (flag==100){
				translate();
				repeat=0;
				signal1 = signal2;
				signal2 = 0;
			}


					fillCircle(x,y,4,BLACK);
					fillCircle(x,y,4,BLUE);	

			
			
		// need to make the count or iTick % by 400 or something to give the signal some time to process correctly
		// Update the ball and paddle in the while loop 
		
		
			
			
			
			
		}
    return(0);
}
//*****************************************************************************