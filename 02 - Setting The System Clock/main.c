//
//  This program shows how you can configure the system clock on the STM8S
//  microcontroller.  The system clock is configured to run at full speed
//	(16Mhz).  A GPIO pin is toggled to give an indication of the system
//	clock speed.
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
#include <intrinsics.h>

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
//  Main program loop.
//
int main(void)
{
    __disable_interrupt();
    //
    //  Initialise Port D.
    //
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR_DDR4 = 1;        //  Port D, bit 4 is output.
    PD_CR1_C14 = 1;         //  Pin is set to Push-Pull mode.
    PD_CR2_C24 = 1;         //  Pin can run upto 10 MHz.
    //
    //  Now setup the system clock
    //
    InitialiseSystemClock();
    __enable_interrupt();
    while (1)
    {
        PD_ODR_ODR4 = 1;    // Turn Port D, Pin 4 on.
        PD_ODR_ODR4 = 0;    // Turn Port D, Pin 4 off.
    }
}
