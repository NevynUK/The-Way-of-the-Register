//
//  This program shows how you can generate a single pulse using
//  timers on the STM8S microcontroller.
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
//  Set up Timer 1, channel 4 to output a single pulse lasting 30 uS.
//
void SetupTimer1()
{
    TIM1_ARRH = 0x03;       //  Reload counter = 960
    TIM1_ARRL = 0xc0;
    TIM1_PSCRH = 0;         //  Prescalar = 0 (i.e. 1)
    TIM1_PSCRL = 0;
    TIM1_CR1_DIR = 0;       //  Up counter.
    TIM1_CR1_CMS = 0;       //  Edge aligned counter.
    TIM1_RCR = 0;           //  No repetition.
    //
    //  Now configure Timer 1, channel 4.
    //
    TIM1_CCMR4_OC4M = 7;    //  Set up to use PWM mode 2.
    TIM1_CCER2_CC4E = 1;    //  Output is enabled.
    TIM1_CCER2_CC4P = 0;    //  Active is defined as high.
    TIM1_CCR4H = 0x01;      //  480 = 50% duty cycle (based on TIM1_ARR).
    TIM1_CCR4L = 0xe0;
    TIM1_BKR_MOE = 1;       //  Enable the main output.
//    TIM1_CR1_OPM = 1;
    TIM1_CR1_CEN = 1;
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
    SetupTimer1();
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
    }
}