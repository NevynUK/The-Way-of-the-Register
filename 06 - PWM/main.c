//
//  This program shows how you can generate a PWM signal on the
//  STM8S microcontroller.
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
    CLK_CKDIVR_HSIDIV = 0;              //  Ensure the clocks are running at full speed.
    CLK_CKDIVR_CPUDIV = 0;
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
//  Setup Timer 2 to PWM signal.
//
void SetupTimer2()
{
    TIM2_PSCR = 0x00;       //  Prescaler = 1.
    TIM2_ARRH = 0x10;       //  High byte of 50,000.
    TIM2_ARRL = 0x00;       //  Low byte of 50,000.
    TIM2_CCR1H = 0x08;      //  High byte of 12,500
    TIM2_CCR1L = 0x00;      //  Low byte of 12,500
    TIM2_CCER1_CC1P = 0;    //  Active high.
    TIM2_CCER1_CC1E = 1;    //  Enable compare mode for channel 1
    TIM2_CCMR1_OC1M = 6;    //  PWM Mode 1 - active if counter < CCR1, inactive otherwise.
    TIM2_CR1_CEN = 1;       //  Finally enable the timer.
}

//
//  Main program loop.
//
void main()
{
    //
    //  Initialise the system.
    //
    __disable_interrupt();
    InitialiseSystemClock();
    SetupTimer2();
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
    }
}