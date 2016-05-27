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
     
	int BUFFERSIZE = 256;
    uint32_t pui32ADC0Value[1];
    int array[128];
	int array2 [128];

    volatile int movingaverage, outputB, outputA;
    volatile int i=0;
    volatile bool printflag = false;
	
	void PrintBuff(void)
	{
		for(int i = 0; i<128; i++)
		{
			UARTprintf("%x ",array[i]);
			if(i % 8 == 0)
			{
				UARTprint("\n");
			}
		}
	}
	
	void SwitchHandler(void)
	{
		GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4)
		printBuff();
		GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
	}
	
    void SS3IntHandler (void) {
    
        ADCIntClear(ADC0_BASE, 3);
	
		ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
		
		array[i]=pui32ADC0Value[0];
		// Storing moving average into an array
	
		 if(i==0)
		 {
			movingaverage=((array[0]+array[127]+array[126]+array[125])/(4));
		 }
		 else if(i==1)
		 {
			movingaverage=((array[1]+array[127]+array[126]+array[0])/(4));
		 }
		 else if(i==2)
		 {
			movingaverage=((array[0]+array[127]+array[1]+array[2])/(4));
		 }
		 else
		 {
			 movingaverage=((pui32ADC0Value[0]+array[i-1]+array[i-2]+array[i-3])/(4));
		 }
		array2[i]=movingaverage;
	
		i++
		if(i == 128)
		{
			i = 0;
			printflag = true;
		}
		
		 // Converting to Hex
		 movingaverage = (movingaverage>>4 & 0xFF);
		 outputB=0xFFF-movingaverage;
		 outputB=(outputB>>4& 0xFF);
		 outputA=movingaverage | 0x0900;
		 outputB=outputB | 0x0A00;
		 
		 // Send Output via SSI - one is an actual reconstruction of a signal and 
		 // the other is a one's complement
		 SSIDataPut(SSI0_BASE, outputA);
		 SSIDataPut(SSI0_BASE, outputB);
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
	
	void ConfigureSwitches(void)
	{
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

		HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
		HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
		HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

		ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);     // PF0, PF4 = switches
		ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU
		ROM_GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_FALLING_EDGE);
		GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
		ROM_IntEnable(INT_GPIOF);
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
		ConfigureSwitches();
		
		// Set up ADC interrupts for Sampling Squencer 3
		ROM_ADCIntEnable(ADC0_BASE, 3);
		ROM_IntEnable(INT_ADC0SS3);
		ROM_IntMasterEnable();          
		
        while(1)
        {
			if(printflag)
			{
				printflag = false;
				printBuff();
			}                   
        }
    }

