#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "utils/cpu_usage.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "driverlib/timer.h"
#include "inc/hw_ssi.h"
#include "driverlib/ssi.h"
#include "Math.h"

#define SYSTICKS_PER_SECOND     100

//*****************************************************************************
//
// The size of the memory transfer source and destination buffers (in words).
//
//*****************************************************************************
#define MEM_BUFFER_SIZE         400


//*****************************************************************************
//
// The source and destination buffers used for memory transfers.
//
//*****************************************************************************
static uint32_t SrcBuf[MEM_BUFFER_SIZE];
static void *SSIpointer = (void*)(SSI0_BASE + SSI_O_DR);



//*****************************************************************************
//
// The count of UART buffers filled, one for each ping-pong buffer.
//
//*****************************************************************************
static uint32_t g_ui32RxBufACount = 0;
static uint32_t g_ui32RxBufBCount = 0;

//*****************************************************************************
//
// The count of memory uDMA transfer blocks.  This value is incremented by the
// uDMA interrupt handler whenever a memory block transfer is completed.
//
//*****************************************************************************
static uint32_t g_ui32MemXferCount = 0;

//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
uint8_t ui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(ui8ControlTable, 1024)
uint8_t ui8ControlTable[1024];
#else
uint8_t ui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1)
    {
		
    }
}
#endif

//*****************************************************************************
//
// The interrupt handler for uDMA interrupts from the memory channel.  This
// interrupt will increment a counter, and then restart another memory
// transfer.
//
//*****************************************************************************
void
uDMAIntHandler(void)
{
    uint32_t ui32Mode;
	
    //
    // Check for the primary control structure to indicate complete.
    //
    ui32Mode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_TMR0A);
		// If it stopped, then it is ready for another transfer, so transfer
		// from the buffer again. 
    if(ui32Mode == UDMA_MODE_STOP)
    {
        g_ui32MemXferCount++;
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_TMR0A, UDMA_MODE_BASIC,
                                     SrcBuf, (void*)(SSI0_BASE + SSI_O_DR),
                                     MEM_BUFFER_SIZE);
        ROM_uDMAChannelEnable(UDMA_CHANNEL_TMR0A);
    }
}

void
Transfer(void)
{
    unsigned int uIdx;

    //
    // Fill the source memory buffer with a simple incrementing pattern.
    //
    for(uIdx = 0; uIdx < 400; uIdx++)
    {
        SrcBuf[uIdx] = 0x0900 + 80* sin((400*3.14*uIdx)+20);
    }
        
    ROM_IntEnable(INT_UDMA);
    ROM_IntEnable(INT_TIMER0A);

    //
    // Put the attributes in a known state for the uDMA software channel.
    // These should already be disabled by default.
    //
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_TMR0A,
                                    UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                                    (UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK));

    //
    // Configure the control parameters for the SW channel.  The SW channel
    // will be used to transfer between two memory buffers, 32 bits at a time.
    // Therefore the data size is 32 bits, and the address increment is 32 bits
    // for both source and destination.  The arbitration size will be set to 8,
    // which causes the uDMA controller to rearbitrate after 8 items are
    // transferred.  This keeps this channel from hogging the uDMA controller
    // once the transfer is started, and allows other channels cycles if they
    // are higher priority.
    //
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_TMR0A | UDMA_PRI_SELECT,
                              UDMA_SIZE_16 | UDMA_SRC_INC_16 | UDMA_DST_INC_NONE |
                              UDMA_ARB_1);

    //
    // Set up the transfer parameters for the software channel.  This will
    // configure the transfer buffers and the transfer size.  Auto mode must be
    // used for software transfers.
    //
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_TMR0A | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC, SrcBuf, (void*)(SSI0_BASE + SSI_O_DR),
                               MEM_BUFFER_SIZE);

    //
    // Now the software channel is primed to start a transfer.  The channel
    // must be enabled.  For software based transfers, a request must be
    // issued.  After this, the uDMA memory transfer begins.
    //
    ROM_uDMAChannelEnable(UDMA_CHANNEL_TMR0A);
}

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
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);

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

void ConfigureSSI(void)
{
	// Initalizes SSI Peripheral
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

  GPIOPinConfigure(GPIO_PA2_SSI0CLK);
  GPIOPinConfigure(GPIO_PA3_SSI0FSS);
  GPIOPinConfigure(GPIO_PA4_SSI0RX);
  GPIOPinConfigure(GPIO_PA5_SSI0TX);

  ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |GPIO_PIN_2);
  ROM_SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,SSI_MODE_MASTER, 1000000, 16);
	
  ROM_SSIEnable(SSI0_BASE);
}
//*****************************************************************************
// Sets up the uDMA and its related peripherals
//*****************************************************************************
void ConfigureuDMA(void)
{
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
		ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		ROM_uDMAEnable();
		ROM_uDMAControlBaseSet(ui8ControlTable);
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	
		//Sets up the Timer to be used by the 
		ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
		ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/1000);

		ROM_IntEnable(INT_TIMER0A);
		ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
		TimerDMAEventSet(TIMER0_BASE, TIMER_DMA_CAPEVENT_A);
		ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
//
// This example demonstrates how to use the uDMA controller to transfer data
// between memory buffers and to and from a peripheral, in this case a UART.
// The uDMA controller is configured to repeatedly transfer a block of data
// from one memory buffer to another.  It is also set up to repeatedly copy a
// block of data from a buffer to the UART output.  The UART data is looped
// back so the same data is received, and the uDMA controlled is configured to
// continuously receive the UART data using ping-pong buffers.
// t3O4XGH 3PFMDZ
// The processor is put to sleep when it is not doing anything, and this allows
// collection of CPU usage data to see how much CPU is being used while the
// data transfers are ongoing.
//
//*****************************************************************************
int
main(void)
{
    ConfigureUART();
    
    ROM_FPULazyStackingEnable();
	
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
	ConfigureuDMA();
	ConfigureSSI();

    ROM_IntMasterEnable();
    Transfer();
    ROM_SSIDMAEnable(SSI0_BASE, SSI_DMA_TX);
	
	// Wait for Interrupts
    ROM_SysCtlSleep();

} 
