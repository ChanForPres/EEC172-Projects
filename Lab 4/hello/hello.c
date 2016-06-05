
/*
	Stanley Chen	
	Kevin Fu 		
	Section 2
	Algorithm:
		- Set up I2C Communication
		- Configure UART Console Communication
		- Set up Interrupts and Enable them
		- Set up OLED Screen
		- On Interrupt
			- Disable & Clear Interrupt
			- Read in Values and store them
			- Reenable Interrupts
			- In Main, we have an infinite loop that:
			- Prints out the Values being read in Accelerometer every .3 seconds
			- Updates the ball location every .3s
				- Add Tilt X/Y values of accelerometer to current X/Y value of ball
				- Make sure to have bounds
*/
#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "utils/uartstdio.h"
#include "inc/hw_nvic.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/ssi.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#ifdef __AVR__
	#include <avr/pgmspace.h>
#endif

// Define Macros for Registers of X, Y and Z
#define XOUT 0x00
#define YOUT 0x01
#define ZOUT 0x02

int WIDTH,HEIGHT;
int _width  , _height  ;
int rotation;
int cursor_y ,cursor_x ; 
int textsize ;
int textcolor ,textbgcolor;
int wrap   ;
// Slave Address Macro
#define SLAVE_ADDRESS 0x4C

// Values that are coming in from the accelerometer 
signed int Ax = 0;
signed int Ay = 0;
signed int Az = 0;
int counter = 0;

// Acceleration Variables to be added to Ball X/Y Value
signed int AccX = 0;
signed int AccY = 0;
signed int AccZ = 1; // Won't use in calculations anywys.

// Ball Logistics - Location of the Ball
int X = 64;
int Y = 64;

// XBEE EXTRA CREDIT
int Data[2];
int ReceiveCount = 0;

void
InitConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // Enable UART0 so that we can configure the clock.
    //    
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	
    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // Select the alternate (UART) function for these pins.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    ROM_UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    UARTStdioConfig(0, 9600, 16000000);
}


void InitEverything(void)
{
		// Enable the GPIO port that is used for the on-board LED.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		//Enable Pin 3 for input
		ROM_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_3);
		/*
			Interrupt is caused on the falling edges and read in from PD3
		*/
		ROM_GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
		GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
		ROM_IntEnable(INT_GPIOD);
	
		/*
		Initialiizing all SSI ports for display
		*/
		
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    // For this example SSI0 is used with PortA[5:2].
		// GPIO port A needs to be enabled so these pins can be used.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
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
    ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);
		ROM_SSIEnable(SSI0_BASE);
		
		// COnfiguring the Data and Reset Lines
		ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);
		reset();
		Adafruit_SSD1351_Init(128,128);
		begin();
		
		// Default Screen of when the game first begins
		fillScreen(BLACK);	
		fillCircle(64,64,4,WHITE);	

		/*
		Set up the Walls of Maze
			
		fillRect(40,0,3,100,WHITE);
		fillRect(70,20,3,108,WHITE);	
		*/
}


//*****************************************************************************

void UART1IntHandler(void) {
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART1_BASE))
    {
			if(ReceiveCount % 2 == 0)
				X = ROM_UARTCharGet(UART1_BASE);
			else
				Y = ROM_UARTCharGet(UART1_BASE);
		}
}

int receive(int r)
{
	`signed int Data;
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, false);
 
    //specify register to be read
    I2CMasterDataPut(I2C0_BASE, r);
 
    //send control byte and register address byte to slave device
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
     
    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));
     
    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
     
    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));
     
    //return data pulled from the specified register
    Data = I2CMasterDataGet(I2C0_BASE);
		
		return Data;
}

// Int Handler to determine the location of hte ball after
// a tilt is detected
void IntHandler(void)
{
	GPIOIntDisable(GPIO_PORTD_BASE, GPIO_INT_PIN_3);
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_3);

	Ax = (receive(XOUT)) & 0x1F;
	Ay = (receive(YOUT))	& 0x1F;
	Az = (receive(ZOUT))	& 0x1F;
	if(Ax & 0x10)
		Ax -= 32;
	if(Ay & 0x10)
		Ay -= 32;
	if(Az & 0x10)
		Az -= 32;
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_3);
}

// Configure all the I2C Registers to enable certain settings
// given by the lab requirements

void configure(void)
{
	// Write to register 8 to set it to 64Hz
	ROM_I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, false);
	ROM_I2CMasterDataPut(I2C0_BASE, 0x08);
	ROM_I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
	while(I2CMasterBusy(I2C0_BASE)){}
	ROM_I2CMasterDataPut(I2C0_BASE, 0x01); 
	ROM_I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
	while(I2CMasterBusy(I2C0_BASE)){} 
	
	//Write to Register 6 to Enable Interrupts
	ROM_I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, false);
	ROM_I2CMasterDataPut(I2C0_BASE, 0x06);
	ROM_I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
	while(I2CMasterBusy(I2C0_BASE)){}
 	ROM_I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
	while(I2CMasterBusy(I2C0_BASE)){}

	//Write to Register 7 to Enable Active Mode
	ROM_I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, false);
	ROM_I2CMasterDataPut(I2C0_BASE, 0x07);
	ROM_I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
	while(I2CMasterBusy(I2C0_BASE)); 
	ROM_I2CMasterDataPut(I2C0_BASE, 0x41); 
	ROM_I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
	while(I2CMasterBusy(I2C0_BASE)); 
}

// Using the tilt value, I will move that many pixels
// I divided by 3 because I didn't want it to move TOO fast.
// Thus, it will slow down the amount of pixels travelled.

void UpdateAccel(void)
{
	AccX = Ax/3;
	AccY = Ay/3;
}

// Updates the location of the ball with the current
// value of the acceleration(velocity)
// Makes sure that there are bounds (EC has wall bounds as well)

void UpdateBall(void)
{
		fillCircle(X,Y,4,BLACK);
		UpdateAccel();
		X -= (2*AccX);
		Y += (2*AccY);
		
		// Boundary Check
		if(X < 5)
		{
			X = 5;
		}
		else if ( X > 126 )
		{
			X = 126;
		}
		if ( Y < 5 )
		{
			Y = 5;
		}
		else if ( Y > 126 )
			Y = 126;
		else if (X<=44 && X>=36 && Y<=102 && Y>=0)
		{
			X+= (2*AccX);
			Y -= 2*AccY;
		}
		else if (X<=74 && X>=66 && Y<=128 && Y>=20){
			
			X += (2*AccX);
			Y -= (2*AccY);
		}
			
		
		fillCircle(X,Y,4,WHITE);
		Data[0] = X;
		Data[1] = Y;
}

int
main(void)
{
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // The I2C0 peripheral must be enabled before use.

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    // This step is not necessary if your part does not support pin muxing.
    //
    ROM_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    ROM_GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    ROM_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    ROM_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    ROM_I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
	InitConsole();
	InitEverything();
	configure();
	ROM_IntMasterEnable();
	while(1)
	{
		
		ROM_SysCtlDelay(SysCtlClockGet()/10);

		UARTprintf("Printing..\n");
		UARTprintf("X Value: %i \n", Ax);
		UARTprintf("Y Value: %i \n", Ay);
		UARTprintf("Z Value: %i \n", Az);

		UpdateBall();
	}
	
    return(0);
}
