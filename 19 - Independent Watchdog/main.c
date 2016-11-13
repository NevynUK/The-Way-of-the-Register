//
//  This program demonstrates how to use the Independent Watchdog timer
//  on the STM8S microcontroller in order to detect software failures and
//  reset the microcontroller.
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
//  Initialise the Independent Watchdog (IWDG)
//
void InitialiseIWDG()
{
    IWDG_KR = 0xcc;         //  Start the independent watchdog.
    IWDG_KR = 0x55;         //  Allow the IWDG registers to be programmed.
    IWDG_PR = 0x02;         //  Prescalar is 2 => each count is 250uS
    IWDG_RLR = 0x04;        //  Reload counter.
    IWDG_KR = 0xaa;         //  Reset the counter.
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
int main()
{
    //
    //  Initialise the system.
    //
    __disable_interrupt();
    InitialiseSystemClock();
    InitialisePorts();
    InitialiseIWDG();
    __enable_interrupt();
    //
    //  Main program loop.
    //
    while (1)
    {
//        IWDG_KR = 0xaa;
        __halt();
    }
}