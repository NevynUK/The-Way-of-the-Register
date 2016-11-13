//
//  This program demonstrates how to use the Auto-Wakeup feature of the STM8S
//  microcontroller.
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
//  Initialise the ports.
//
//  Configure all of Port D for output.
//
void InitialisePorts()
{
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR = 0xff;          //  All pins are outputs.
    PD_CR1 = 0xff;          //  Push-Pull outputs.
    PD_CR2 = 0xff;          //  Output speeds up to 10 MHz.
}

//--------------------------------------------------------------------------------
//
//  Initialise the Auto Wake-up feature.
//
//
//
void InitialiseAWU()
{
    AWU_CSR1_AWUEN = 0;     // Disable the Auto-wakeup feaure.
    AWU_APR_APR = 38;
    AWU_TBR_AWUTB = 1;
    AWU_CSR1_AWUEN = 1;     // Enable the Auto-wakeup feaure.
}

//--------------------------------------------------------------------------------
//
//  Auto Wakeup Interrupt Service Routine (ISR).
//
//  Pulse PD0 for a short time.  The NOP instructions generate a more stable
//  pulse on the oscilloscope.
//
#pragma vector = AWU_vector
__interrupt void AWU_IRQHandler(void)
{
    volatile unsigned char reg;

    PD_ODR = 1;
    asm("nop;");
    asm("nop;");
    PD_ODR = 0;
    reg = AWU_CSR1;     // Reading AWU_CSR1 register clears the interrupt flag.
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
    InitialisePorts();
    InitialiseAWU();
    __enable_interrupt();
    //
    //  Main program loop.
    //
    while (1)
    {
        __halt();
    }
}