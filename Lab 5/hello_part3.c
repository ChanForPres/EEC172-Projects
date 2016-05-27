    #include <stdbool.h>
    #include <stdint.h>
    #include "inc/hw_memmap.h"
    #include "inc/hw_types.h"
    #include "inc/hw_gpio.h"
    #include "inc/hw_ints.h"
    #include "inc/hw_nvic.h"
    #include "driverlib/fpu.h"
    #include "driverlib/adc.h"
    #include "driverlib/gpio.h"
    #include "driverlib/pin_map.h"
    #include "driverlib/debug.h"
    #include "driverlib/sysctl.h"
    #include "driverlib/uart.h"
    #include "driverlib/rom.h"
    #include "utils/uartstdio.h"
    #include "driverlib/timer.h"
    #include "driverlib/interrupt.h"
    #include "driverlib/ssi.h"
	
    // Sets up the Constants for the Program
	const int BUFFERSIZE = 128;
	const int rate = 90000;
	
	// Set up the Buffers to be used:
	// pui32ADC0Value gets the value from ADC
	// array stores all the values that are retrieved from ADC
	// Ping and Pong Buffers used for PingPong Replication
	// BufferUsed* points at the Ping or Pong Buffer to be used
	// various flags are set to determine which buffer to use and
	// to check if there was swaps
    uint32_t pui32ADC0Value[1];
    int array[BUFFERSIZE];
	uint32_t PongBuffer[BUFFERSIZE];
	uint32_t PingBuffer[BUFFERSIZE];
	uint32_t* BufferUsed;
	volatile bool useping = true, usepong = false,swapflag = false;
	
    volatile int i=0;
	
	/*
		Algorithm:
			Create a threshold value that it cannot pass
			Increment a counter to use to determine how many of these values lie above the threshold
				Higher amount of values above threshold means that its sine
				Lower amount of values above threshold means its triangle
				if the slope is greater than a certain value then it is square
			frequency is calculated by dividing the sampling rate by the period between the two points
	*/
	void Analyze(void)
	{
		ROM_IntDisable(INT_ADC0SS3);
		int counter = 0, High1 = 0, High2 = 0;
		int point1 = 0, point2 = 0, freq = 0, period;
		for(int count = 0; count < 128; count++)
		{
			if(BufferUsed[count] > 3500)
			{
				counter++;
			}
			High1 = High2;
			if(count == 1)
			{
				High2 = (BufferUsed[count]-BufferUsed[count-1])/2;
			}
			if(High1<0 && High2>0)
			{
				point1 = count;
			}
			else if(High1>0 && High2<0)
			{
				point2 = count;
			}
			else if(point1 > 0 && point2 > 0)
			{
				period = point2 - point1;
				freq = rate/period;
				UARTprintf("Frequency: %d         ",freq);
				if(currentSlope > 1200)
				{
					UARTprintf("Square Wave\n");
				}
				else if( counter < 30)
				{
					UARTprintf("Triangle Wave\n");
				}
				else
				{
					UARTprintf("Sine Wave\n");
				}
			}
		}
		ROM_IntEnable(INT_ADC0SS3);
	}
	/*
		Algorithm:
			Clears Interrupt
			Grabs the value from the ADC FIFO and puts it into the array size of 1
			store the value into an 128 array
			once you're full
				set the swapflag to be true - if swapflag is true, run Analyze
				useping (naturally true) - allows the ping buffer to be used and store all of hte array
				into the PingBuffer and point the BufferUsed at Ping
				usepong is the same idea except it works for pong buffer while the ping buffer is busy
			
	*/
    void SS3IntHandler (void) {
    
        ADCIntClear(ADC0_BASE, 3);
		ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
		array[i]=pui32ADC0Value[0];
		i++
		
		if(i == 128)
		{
			i = 0;
			swapflag = true;
			for(int j = 0; j < 128; j++)
			{
				if(useping)
				{
					useping = false;
					usepong = true;
					PingBuffer[j] = array[j];
					BufferUsed = PingBuffer;
				}
				else
				{
					useping = true;
					usepong = false;
					PongBuffer[j] = array[j];
					BufferUsed = PongBuffer;
				}
			}
		}
		
}

    void ConfigureUART0(void) {
        //
        // Enable the GPIO Peripheral used by the UART0 and enable UART0
        //
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);  
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
        // Initialize UART0 for console I/O. 
        //
        UARTStdioConfig(0, 9600, 16000000);
    }
	
	void ConfigureSSI(void)
	{
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
     
        // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
        //
        GPIOPinConfigure(GPIO_PA2_SSI0CLK);
        GPIOPinConfigure(GPIO_PA3_SSI0FSS);
        GPIOPinConfigure(GPIO_PA4_SSI0RX);
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
        ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
                       GPIO_PIN_2);
     
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
                           SSI_MODE_MASTER, 1000000, 16);
     
        //
        // Enable the SSI0 module.
        //
        ROM_SSIEnable(SSI0_BASE);
	}
	
	void ConfigureADC(void)
	{
		 ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);      
     
        //
        // The ADC0 peripheral must be enabled for use.
        //
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
     
        //
        // For this example ADC0 is used with AIN0 on port E3.
        // GPIO port E needs to be enabled so these pins can be used.
        //
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
     
        //
        // Select the analog ADC function for these pins.
        // Consult the data sheet to see which functions are allocated per pin.
        //
        ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
     
        //
        // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
        // will do a single sample when the processor sends a signal to start the
        // conversion.  Each ADC module has 4 programmable sequences, sequence 0
        // to sequence 3.  This example is arbitrarily using sequence 3.
        //
        ROM_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
     
        //
        // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
        // single-ended mode (default) and configure the interrupt flag
        // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
        // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
        // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
        // sequence 0 has 8 programmable steps.  Since we are only doing a single
        // conversion using sequence 3 we will only configure step 0.  For more
        // information on the ADC sequences and steps, reference the datasheet.
        //
        ROM_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);
        //
        // Since sample sequence 3 is now configured, it must be enabled.
        //
        ROM_ADCSequenceEnable(ADC0_BASE, 3);
        //
        // Clear the interrupt status flag.  This is done to make sure the
        // interrupt flag is cleared before we sample.
        //
        ROM_ADCIntClear(ADC0_BASE, 3);
		
		ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
		ROM_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
		ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/4000);
		ROM_TimerEnable(TIMER0_BASE, TIMER_A);
	}
	
    int
    main(void)
    {
        ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
     
		// Configure UART, SSI, and the Switches for interrupts
		ConfigureUART0();
		ConfigureSSI();
		
		// Set up ADC interrupts for Sampling Squencer 3
		ROM_ADCIntEnable(ADC0_BASE, 3);
		ROM_IntEnable(INT_ADC0SS3);
		ROM_IntMasterEnable();          
		
		// Wait for Interrupt, check if there was a swap if there was then run Analyze
        while(1)
        {
			ROM_SysCtlSleep();
			SysCtlDelay(SysCtlClockGet()/10);
			if (swapflag)
			{
				swapflag = false;
				Analyze();
			}
                                   
        }
    }

