//
//  This application implements a GoBus 1.0 driver on the STM8S
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
#if defined(DISCOVERY)
    #include <iostm8S105c6.h>
#else
    #include <iostm8s103f3.h>
#endif

#include <intrinsics.h>

//--------------------------------------------------------------------------------
//
//  Function table structure.
//
typedef struct
{
    unsigned char command;          //  Command number.
    void (*functionPointer)();      //  Pointer to the function to be executed.
} FunctionTableEntry;

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

//--------------------------------------------------------------------------------
//
//  Pins used for diagnostic output
//
#if defined(DISCOVERY)
    #define PIN_STATUS_CODE         PD_ODR_ODR2
#else
    #define PIN_STATUS_CODE         PD_ODR_ODR6
#endif
#define PIN_BIT_BANG_CLOCK      PD_ODR_ODR4
#define PIN_BIT_BANG_DATA       PD_ODR_ODR5
//
//  Pin to notify the GO main board that we have some data ready for processing.
//
#if defined(DISCOVERY)
    #define PIN_GOBUS_INTERRUPT     PD_ODR_ODR1
#else
    #define PIN_GOBUS_INTERRUPT     PD_ODR_ODR2
#endif
//
//  SPI Chip select IRQ and vector information.
//
#if defined(DISCOVERY)
    #define SPI_CHIP_SELECT_VECTOR      6
    #define SPI_CS_IRQ_DIRECTION        EXTI_CR1_PBIS
#else
    #define SPI_CHIP_SELECT_VECTOR      5
    #define SPI_CS_IRQ_DIRECTION        EXTI_CR1_PAIS
#endif
//
//  Constants having a special meaning in the GoBus 1.0 protocol.
//
#define GO_COMMAND_RESPONSE     0x80
#define GO_FRAME_PREFIX         0x2a
#define GO_MODULE_ID_REQUEST    0xfe
#define GO_BUFFER_SIZE          17

//--------------------------------------------------------------------------------
//
//  Application global variables.
//
unsigned char _rxBuffer[GO_BUFFER_SIZE + 1];    // Buffer holding the received data plus a CRC.
unsigned char _txBuffer[GO_BUFFER_SIZE];        // Buffer holding the data to send.
unsigned char *_rx;                             // Place to put the next byte received.
unsigned char *_tx;                             // Next byte to send.
int _rxCount;                                   // Number of characters received.
int _txCount;                                   // Number of characters sent.
volatile int _status;                           // Application status code.
//
//  GUID which identifies this module.
//
unsigned char _moduleID[] = { 0x80, 0x39, 0xe8, 0x2b, 0x55, 0x58, 0xeb, 0x48,
                              0xab, 0x9e, 0x48, 0xd3, 0xfd, 0xae, 0x8c, 0xee };
//
//  Forward function declarations for the function table.
//
void AddFive();
void GetValue();
//
//  Table of pointers to functions which implement the specified commands.
//
FunctionTableEntry _functionTable[] = { { 0x01, AddFive }, { 0x02, GetValue } };
//
//  Number of functions in the function table.
//
const int _numberOfFunctions = sizeof(_functionTable) / sizeof(FunctionTableEntry);

//--------------------------------------------------------------------------------
//
//  Raise an interrupt to the GO! main board to indicate that there is some data
//  ready for collection.  The IRQ on the GO! board is configured as follows:
//
//  _irqPort = new InterruptPort((Cpu.Pin) socketGpioPin, false, Port.ResistorMode.PullUp,
//                               Port.InterruptMode.InterruptEdgeLow);
//
void NotifyGOBoard()
{
    PIN_GOBUS_INTERRUPT = 0;
    __no_operation();
    PIN_GOBUS_INTERRUPT = 1;
}

//--------------------------------------------------------------------------------
//
//  GO! function 1 - add 5 to byte 2 in the Rx buffer and put the answer into the
//  Tx buffer.
//
void AddFive()
{
    _txBuffer[1] = _rxBuffer[2] + 5;
    NotifyGOBoard();
}

//--------------------------------------------------------------------------------
//
//  GO! Function 2 - return the Tx buffer back to the GO! board.
//
void GetValue()
{
    NotifyGOBoard();
}

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
//  This method resets SPI ready for the next transmission/reception of data
//  on the GO! bus.
//
//  Do not call this method whilst SPI is enabled as it will have no effect.
//
void ResetGoFrame()
{
    if (!SPI_CR1_SPE)
    {
        //
        //  The following two lines reset an overflow error condition
        //  if it exists.
        //
        (void) SPI_DR;
        (void) SPI_SR;
        //
        //  Get ready to send any data.  The first byte of the frame will be
        //  the predefined prefix frame.  We get ready to send the module ID
        //  in case this is being requested.
        //
        SPI_DR = GO_FRAME_PREFIX;               //  First byte of the response.
        _txBuffer[0] = _moduleID[0];            //  Second byte in the response.
        //
        //  Now reset the buffer pointers and counters ready for data transfer.
        //
        _rx = _rxBuffer;
        _tx = _txBuffer;
        _rxCount = 0;
        _txCount = 0;
        //
        //  Note the documentation states this should be SPI_CR2_CRCEN
        //  but the header files have SPI_CR_CECEN defined.  I have only
        //  checked the STM8S103f3 and STM8S105c6 headers but this appears
        //  to be a consistent error.
        //
        SPI_CR2_CECEN = 0;                      //  Reset the CRC calculation.
        SPI_CR2_CRCNEXT = 0;
        SPI_CR2_CECEN = 1;
        SPI_SR_CRCERR = 0;
    }
}

//--------------------------------------------------------------------------------
//
//  SPI chip select interrupt service routine.
//
#pragma vector = SPI_CHIP_SELECT_VECTOR
__interrupt void EXTI_SPI_CS_PORT_IRQHandler(void)
{
    #if defined (DEBUG)
        PIN_STATUS_CODE = 1;
    #endif
    if (SPI_CS_IRQ_DIRECTION == 2)
    {
        //
        //  Transition from high to low selects this slave device.
        //
        SPI_CR2_SSI = 0;
        SPI_CR1_SPE = 1;                        //  Enable SPI.
        #if defined (DEBUG)
            PIN_STATUS_CODE = 0;
        #endif
        SPI_CS_IRQ_DIRECTION = 1;               //  Waiting for rising edge next.
    }
    else
    {
        //
        //  Transition from low to high disables SPI
        //
        SPI_CR1_SPE = 0;                        //  Disable SPI.
        SPI_CR2_SSI = 1;
        SPI_CS_IRQ_DIRECTION = 2;               //  Waiting for falling edge next.
        ResetGoFrame();
        #if defined (DEBUG)
            PIN_STATUS_CODE = 0;
        #endif
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
        SPI_CR1_SPE = 0;
        _status = SC_OVERFLOW;
        return;
    }
    //
    //  Check to see if we have a CRC error.
    //
    if (SPI_SR_CRCERR)
    {
        SPI_CR1_SPE = 0;
        _status = SC_CRC_ERROR;
        return;
    }
    //
    //  Looks like we have a valid transmit/receive interrupt.
    //
    if (SPI_SR_TXE)
    {
        //
        //  The master is ready to receive another byte.
        //
        if (_txCount < (GO_BUFFER_SIZE - 1))
        {
            SPI_DR = *_tx;
            _tx++;
            _txCount++;
            if (_txCount == (GO_BUFFER_SIZE - 1))
            {
                SPI_CR2_CRCNEXT = 1;
            }
        }
    }
    if (SPI_SR_RXNE)
    {
        if (_rxCount < (GO_BUFFER_SIZE + 1))
        {
            //
            //  We have received some data and we have space in the buffer.
            //
            if ((_rxCount == 0) && (SPI_DR == GO_MODULE_ID_REQUEST))
            {
                _tx = _moduleID;
                _tx++;
            }
            *_rx = SPI_DR;              //  Read the byte we have received.
            _rx++;
            _rxCount++;
            if (_rxCount == (GO_BUFFER_SIZE - 1))
            {
                _status = SC_RX_BUFFER_FULL;
            }
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
    CLK_CCOR = 1;                       //  Turn off CCO.
    CLK_HSITRIMR = 0;                   //  Turn off any HSIU trimming.
    CLK_SWIMCCR = 0;                    //  Set SWIM to run at clock / 2.
    CLK_SWR = 0xe1;                     //  Use HSI as the clock source.
    CLK_SWCR = 0;                       //  Reset the clock switch control register.
    CLK_SWCR_SWEN = 1;                  //  Enable switching.
    while (CLK_SWCR_SWBSY != 0);        //  Pause while the clock switch is busy.
}

//--------------------------------------------------------------------------------
//
//  Initialise SPI to be a slave device.
//
void InitialiseSPIAsSlave()
{
    SPI_CR1_SPE = 0;                    //  Disable SPI.
    SPI_CR1_CPOL = 0;                   //  Clock is low when idle.
    SPI_CR1_CPHA = 1;                   //  Sample the data on the falling edge.
    SPI_ICR_TXIE = 1;                   //  Enable the SPI TXE interrupt.
    SPI_ICR_RXIE = 1;                   //  Enable the SPI RXE interrupt.
    SPI_ICR_ERRIE = 1;                  //  Enable the gernation of interrupts on errors.
    SPI_CR2_SSI = 0;                    //  This is SPI slave device.
    SPI_CR2_SSM = 1;                    //  Slave management performed by software.

    PC_CR1_C17 = 1;                     //  Configure MISO as input, pull-up, no interrupt.
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
    //  Initialise the CS port for input and set up the interrupt behaviour.
    //
#if defined(DISCOVERY)
    PB_ODR = 0;             //  Turn the outputs off.
    PB_DDR = 0;             //  All pins are inputs.
    PB_CR1 = 0xff;          //  All inputs have pull-ups enabled.
    PB_CR2 = 0xff;          //  Interrupts enabled on all pins.
    EXTI_CR1_PBIS = 2;      //  Port B interrupt on falling edge (initially).
#else
    PA_ODR = 0;             //  Turn the outputs off.
    PA_DDR = 0;             //  All pins are inputs.
    PA_CR1 = 0xff;          //  All inputs have pull-ups enabled.
    PA_CR2 = 0xff;          //  Interrupts enabled on all pins.
    EXTI_CR1_PAIS = 2;      //  Port A interrupt on falling edge (initially).
#endif
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
    ResetGoFrame();
    InitialisePorts();
    _status = SC_UNKNOWN;
    __enable_interrupt();
    //
    //  Main program loop.
    //
    while (1)
    {
        __wait_for_interrupt();
        if ((_status == SC_RX_BUFFER_FULL) && (_rxBuffer[0] == GO_COMMAND_RESPONSE))
        {
            #if defined(DEBUG)
                BitBangBuffer(_rxBuffer, GO_BUFFER_SIZE);
            #endif
            //
            //  Work out which function to call.
            //
            if (_numberOfFunctions > 0)
            {
                for (int index = 0; index < _numberOfFunctions; index++)
                {
                    if (_functionTable[index].command == _rxBuffer[1])
                    {
                        (*(_functionTable[index].functionPointer))();
                        break;
                    }
                }
            }
        }
        _status = SC_UNKNOWN;
    }
}