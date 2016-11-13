//
//  Simple SPI slave application.  This application takes the
//  data coming in through the MOSI pin and buffers the data
//  ready for processing.
//
//  This software is provided under the CC BY-SA 3.0 licence.  A
//  copy of this licence can be found at:
//
//  http://creativecommons.org/licenses/by-sa/3.0/legalcode
//
#include <iostm8S105c6.h>
#include <intrinsics.h>

//--------------------------------------------------------------------------------
//
//  Define the status codes.
//
#define SC_UNKNOWN          0
#define SC_OK               1
#define SC_RX_BUFFER_FULL   2
#define SC_TX_BUFFER_EMPTY  3
#define SC_OVERFLOW         4
#define SC_CRC_ERROR        5
#define SC_CS_FALLING_EDGE  6
#define SC_CS_RISING_EDGE   7

//--------------------------------------------------------------------------------
//
//  Pins used for diagnostic output
//
#define PIN_STATUS_CODE         PD_ODR_ODR2
#define PIN_BIT_BANG_CLOCK      PD_ODR_ODR4
#define PIN_BIT_BANG_DATA       PD_ODR_ODR6

//--------------------------------------------------------------------------------
//
//  Miscellaneous constants
//
#define BUFFER_SIZE             17

//--------------------------------------------------------------------------------
//
//  Application global variables.
//
unsigned char _rxBuffer[BUFFER_SIZE];       // Buffer holding the received data.
unsigned char _txBuffer[BUFFER_SIZE];       // Buffer holding the data to send.
unsigned char *_rx;                         // Place to put the next byte received.
unsigned char *_tx;                         // Next byte to send.
int _rxCount;                               // Number of characters received.
int _txCount;                               // Number of characters sent.
int _status;                                // Application status code.

//--------------------------------------------------------------------------------
//
//  Output status code on the status pin.
//
void OutputStatusCode(int code)
{
    for (unsigned short index = 0; index < code; index++)
    {
        PIN_STATUS_CODE = 1;
        __no_operation();
        PIN_STATUS_CODE = 0;
    }
}

//--------------------------------------------------------------------------------
//
//  Bit bang data on the diagnostic pins.
//
void BitBang(unsigned char byte)
{
    for (short bit = 7; bit >= 0; bit--)
    {
        if (byte & (1 << bit))
        {
            PIN_BIT_BANG_DATA = 1;
        }
        else
        {
            PIN_BIT_BANG_DATA = 0;
        }
        PIN_BIT_BANG_CLOCK = 1;
        __no_operation();
        PIN_BIT_BANG_CLOCK = 0;
    }
    PIN_BIT_BANG_DATA = 0;
}

//--------------------------------------------------------------------------------
//
//  Bit bang a buffer of data on the diagnostic pins.
//
void BitBangBuffer(unsigned char *buffer, int size)
{
    for (int index = 0; index < size; index++)
    {
        BitBang(buffer[index]);
    }
}

//--------------------------------------------------------------------------------
//
//  Reset the SPI buffers and pointers to their default values.
//
void ResetSPIBuffers()
{
    SPI_DR = 0xff;
    _rxCount = 0;
    _txCount = 0;
    _rx = _rxBuffer;
    _tx = _txBuffer;
}

//--------------------------------------------------------------------------------
//
//  SPI chip select interrupt service routine.
//
#pragma vector = 6
__interrupt void EXTI_PORTB_IRQHandler(void)
{
//    if (EXTI_CR1_PBIS != 1)
    PIN_STATUS_CODE = 1;
    if (EXTI_CR1_PBIS == 2)
    {
        //
        //  Transition from high to low selects this slave device.
        //
//        ResetSPIBuffers();
//        (void) SPI_DR;
//        (void) SPI_SR;
//        SPI_DR = *_tx++;                        //  Load the transmit with first byte.
//        _txCount++;
        SPI_CR2_SSI = 0;
        SPI_CR1_MSTR = 0;
        SPI_CR1_SPE = 1;                        // Enable SPI.
        PIN_STATUS_CODE = 0;
        EXTI_CR1_PBIS = 1;                      //  Waiting for rising edge next.
    }
    else
    {
        //
        //  Transition from low to high disables SPI
        //
        SPI_CR1_SPE = 0;                        //  Disable SPI.
        SPI_CR2_SSI = 1;
        PIN_STATUS_CODE = 0;
        EXTI_CR1_PBIS = 2;                      //  Waiting for falling edge next.
        ResetSPIBuffers();
    }
}

//--------------------------------------------------------------------------------
//
//  SPI Interrupt service routine.
//
#pragma vector = SPI_TXE_vector
__interrupt void SPI_IRQHandler(void)
{
    //
    //  Check for an overflow error.
    //
    if (SPI_SR_OVR)
    {
        (void) SPI_DR;                      // These two reads clear the overflow
        (void) SPI_SR;                      // error.
        _status = SC_OVERFLOW;
        OutputStatusCode(_status);
        return;
    }
    //
    //  Looks like we have a valid transmit/receive interrupt.
    //
    if (SPI_SR_RXNE)
    {
        //
        //  We have received some data.
        //
        *_rx = SPI_DR;              //  Read the byte we have received.
        _rx++;
        _rxCount++;
        if (_rxCount == BUFFER_SIZE)
        {
            _status = SC_RX_BUFFER_FULL;
            OutputStatusCode(_status);
            _rx = _rxBuffer;
            _rxCount = 0;
        }
    }
    if (SPI_SR_TXE)
    {
        //
        //  The master is ready to receive another byte.
        //
        SPI_DR = *_tx;
        _tx++;
        _txCount++;
        if (_txCount == BUFFER_SIZE)
        {
            OutputStatusCode(SC_TX_BUFFER_EMPTY);
            _tx = _txBuffer;
            _txCount = 0;
        }
    }
}

//--------------------------------------------------------------------------------
//
//  Setup the system clock to run at 16MHz using the internal oscillator.
//
void InitialiseSystemClock()
{
    CLK_ICKR = 0;                       //  Reset the Internal Clock Register.
    CLK_ICKR_HSIEN = 1;                 //  Enable the HSI.
    CLK_ECKR = 0;                       //  Disable the external clock.
    while (CLK_ICKR_HSIRDY == 0);       //  Wait for the HSI to be ready for use.
    CLK_CKDIVR = 0;                     //  Ensure the clocks are running at full speed.
    CLK_PCKENR1 = 0xff;                 //  Enable all peripheral clocks.
    CLK_PCKENR2 = 0xff;                 //  Ditto.
    CLK_CCOR = 0;                       //  Turn off CCO.
    CLK_HSITRIMR = 0;                   //  Turn off any HSIU trimming.
    CLK_SWIMCCR = 0;                    //  Set SWIM to run at clock / 2.
    CLK_SWR = 0xe1;                     //  Use HSI as the clock source.
    CLK_SWCR = 0;                       //  Reset the clock switch control register.
    CLK_SWCR_SWEN = 1;                  //  Enable switching.
    while (CLK_SWCR_SWBSY != 0);        //  Pause while the clock switch is busy.
}

//--------------------------------------------------------------------------------
//
//  Initialise SPI to be SPI slave.
//
void InitialiseSPIAsSlave()
{
    SPI_CR1_SPE = 0;                    //  Disable SPI.
    SPI_CR1_CPOL = 0;                   //  Clock is low when idle.
    SPI_CR1_CPHA = 1;                   //  Sample the data on the falling edge.
    SPI_ICR_TXIE = 1;                   //  Enable the SPI TXE interrupt.
    SPI_ICR_RXIE = 1;                   //  Enable the SPI RXE interrupt.
    SPI_CR2_SSI = 0;                    //  This is SPI slave device.
    SPI_CR2_SSM = 1;                    //  Slave management performed by software.
    SPI_CR1_MSTR = 0;
}

//--------------------------------------------------------------------------------
//
//  Initialise the ports.
//
void InitialisePorts()
{
    //
    //  Initialise Port D for debug output.
    //
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR = 0xff;          //  All pins are outputs.
    PD_CR1 = 0xff;          //  Push-Pull outputs.
    PD_CR2 = 0xff;          //  Output speeds upto 10 MHz.
    //
    //  Initialise Port B for input.
    //
    PB_ODR = 0;             //  Turn the outputs off.
    PB_DDR = 0;             //  All pins are inputs.
    PB_CR1 = 0xff;          //  All inputs have pull-ups enabled.
    PB_CR2 = 0xff;          //  Interrupts enabled on all pins.
    //
    //  Now set up the interrupt behaviour.
    //
    EXTI_CR1_PBIS = 2;      //  Port B interrupt on falling edge.
    ITC_SPR2_VECT6SPR = 1;  //  Interrupt Priority 1 for Port B intrrupt.
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
int main(void)
{
    //
    //  Initialise the system.
    //
    __disable_interrupt();
    InitialiseSystemClock();
    InitialiseSPIAsSlave();
    ResetSPIBuffers();
    for (unsigned char index = 0; index < BUFFER_SIZE; index++)
    {
        _txBuffer[index] = index + 100;
    }
    InitialisePorts();
    _status = SC_UNKNOWN;
    __enable_interrupt();
    //
    //  Main program loop.
    //
    while (1)
    {
        __wait_for_interrupt();
        if (_status == SC_RX_BUFFER_FULL)
        {
            BitBangBuffer(_rxBuffer, BUFFER_SIZE);
        }
        _status = SC_UNKNOWN;
    }
}