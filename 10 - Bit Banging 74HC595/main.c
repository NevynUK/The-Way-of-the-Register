//
//  Illustrate how we can control the 74HC595 using the STM8S by
//  bit banging the data on an output port.
//
//  This software is provided under the CC BY-SA 3.0 licence.  A
//  copy of this licence can be found at:
//
//  http://creativecommons.org/licenses/by-sa/3.0/legalcode
//
#if defined DISCOVERY
    #include <iostm8S105c6.h>
#elif defined PROTOMODULE
    #include <iostm8s103k3.h>
#else
    #include <iostm8s103f3.h>
#endif
#include <stdlib.h>
#include <intrinsics.h>

//
//  Define the pins which we will be using to control the shift registers.
//
#define SR_CLOCK            PD_ODR_ODR3
#define SR_DATA             PD_ODR_ODR2
#define SR_CLEAR            PC_ODR_ODR3
#define SR_OUTPUT_ENABLE    PC_ODR_ODR4
#define SR_LATCH            PD_ODR_ODR4

//
//  Data area holding the values in the register.
//
unsigned char *registers;                          //  Data in the registers.
unsigned char numberOfRegisters;                   //  Number of registers in the chain.

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

//
//  BitBang the data through the GPIO ports.
//
void OutputData(unsigned char *data, int numberOfBytes)
{
    //
    //  Initialise the shift register by turning off the outputs, clearing
    //  the registers and setting the clock and data lines into known states.
    //
    SR_OUTPUT_ENABLE = 1;               //  Turn off the outputs.
    SR_LATCH = 0;                       //  Ready for latching the shift register into the storage register.
    SR_DATA = 0;                        //  Set the data line low.
    SR_CLOCK = 0;                       //  Set the clock low.
    SR_CLEAR = 0;                       //  Clear the shift registers.
    __no_operation();
    SR_CLEAR = 1;
    //
    //  Output the data.
    //
    for (int currentByte = 0; currentByte < numberOfBytes; currentByte++)
    {
        unsigned char b = data[currentByte];
        for (int index = 7; index >= 0; index--)
        {
            SR_DATA = ((b >> index) & 0x01);
            SR_CLOCK = 1;               //  Send a clock pulse.
            __no_operation();
            SR_CLOCK = 0;
        }
    }
    //
    //  Set the clock line into a known state and enable the outputs.
    //
    SR_CLOCK = 0;                       //  Set the clock low.
    SR_LATCH = 1;                       //  Transfer the data from the shift register into the storage register.
    SR_OUTPUT_ENABLE = 0;               //  Turn on the outputs.
}

//
//  Clear all of the bytes in the registers to 0.
//
void ClearRegisters(unsigned char *registers, unsigned char numberOfBytes)
{
    for (unsigned char index = 0; index < numberOfBytes; index++)
    {
        registers[index] = 0;
    }
}

//
//  Main program loop.
//
int main(void)
{
    __disable_interrupt();
    //
    //  PD3 and PD2 are used for the serial data going to the registers.
    //  Configure Port D for output.
    //
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR = 0xff;          //  All pins are outputs.
    PD_CR1 = 0xff;          //  Push-Pull outputs.
    PD_CR2 = 0xff;          //  Output speeds upto 10 MHz.
    //
    //  PB4 and PB5 are used for control of the output and clearing the
    //  registers.  Configure Port B for output.
    //
    PC_ODR = 0;             //  All pins are turned off.
    PC_DDR = 0xff;          //  All pins are outputs.
    PC_CR1 = 0xff;          //  Push-Pull outputs.
    PC_CR2 = 0xff;          //  Output speeds upto 10 MHz.
    //
    //  Now setup the system clock
    //
    InitialiseSystemClock();
    __enable_interrupt();
    //
    //  Main loop really starts here.
    //
    numberOfRegisters = 2;
    registers = (unsigned char *) malloc(numberOfRegisters);
    ClearRegisters(registers, numberOfRegisters);
    while (1)
    {
        for (int c = 0; c < 16; c++)
        {
            if (c < 8)
            {
                registers[0] = (unsigned char) ((1 << c) & 0xff);
                registers[1] = 0;
            }
            else
            {
                registers[0] = 0;
                registers[1] = (unsigned char) ((1 << (c - 8)) & 0xff);
            }
            OutputData(registers, 2);
            for (unsigned long counter = 0; counter < 100000; counter++);
        }
    }
}