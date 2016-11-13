//
//  This program demonstrates how to use the Window Watchdog on the STM8S
//  microcontroller.
//
//  This software is provided under the CC BY-SA 3.0 licence.  A
//  copy of this licence can be found at:
//
//  http://creativecommons.org/licenses/by-sa/3.0/legalcode
//
#include <iostm8S105c6.h>
#include <intrinsics.h>

//
//  Clock frequency used in the calculations.
//
#define CLOCK_FREQUENCY     16000000
#define PRESCALER           8

#define SR_CLOCK            PD_ODR_ODR5
#define SR_DATA             PD_ODR_ODR3

//--------------------------------------------------------------------------------
//
//  BitBang the data through the GPIO ports.
//
void BitBangByte(unsigned char b)
{
    //
    //  Initialise the shift register by turning off the outputs, clearing
    //  the registers and setting the clock and data lines into known states.
    //
    SR_DATA = 0;                        //  Set the data line low.
    SR_CLOCK = 0;                       //  Set the clock low.
    //
    //  Output the data.
    //
    for (int index = 7; index >= 0; index--)
    {
        SR_DATA = ((b >> index) & 0x01);
        SR_CLOCK = 1;               //  Send a clock pulse.
        __no_operation();
        SR_CLOCK = 0;
    }
    //
    //  Set the clock line into a known state and enable the outputs.
    //
    SR_CLOCK = 0;                       //  Set the clock low.
    SR_DATA = 0;
}

//--------------------------------------------------------------------------------
//
//  Measure the frequency of the signal connected to Timer 1 Channel 3.
//
unsigned int MeasureFrequency()
{
    unsigned int ICValue1 = 0, ICValue2 = 0;
    unsigned char  high1, low1, high2, low2;

    TIM1_CR1_OPM = 0;
    TIM1_SR1 = 0x1E;
    while (!TIM1_SR1_CC3IF);
    TIM1_CR1_OPM = 0;
    TIM1_SR1 = 0x1E;
    while (!TIM1_SR1_CC3IF);

    high1 = TIM1_CCR3H;
    low1 = TIM1_CCR3L;
    //
    //  Clear the capture compare flag and restart.
    //
    TIM1_CR1_OPM = 0;
    TIM1_SR1 = 0x1E;

    while (!TIM1_SR1_CC3IF);

    high2 = TIM1_CCR3H;
    low2 = TIM1_CCR3L;

    ICValue1 = (high1 << 8) | low1;
    ICValue2 = (((unsigned short) high2) << 8) | low2;
    //
    //  Now calculate the frequency.
    //
    return((PRESCALER * CLOCK_FREQUENCY) / (ICValue2 - ICValue1));
}

//--------------------------------------------------------------------------------
//
//  Timer 2 Overflow handler.
//
#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_UPD_OVF_IRQHandler(void)
{
    PD_ODR_ODR6 = !PD_ODR_ODR6;
    TIM2_SR1_UIF = 0;       //  Reset the interrupt otherwise it will fire again straight away.
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
//  Initialise Timer 1, setting up for Capture Compare on Channel 3.
//
void InitialiseTimer1()
{
    //
    //  Configure Timer 1, Channel 3 for Capture/Compare.
    //
    TIM1_CR1_CEN = 0;
    TIM1_CCMR3_CC3S = 1;
    TIM1_CCMR3_OC3FE = 1;
    TIM1_CCMR3_OC3PE = 1;
    TIM1_CCER2_CC3E = 1;
    TIM1_CCER2_CC3P = 1;
    TIM1_CR1_CEN = 1;
}

//--------------------------------------------------------------------------------
//
//  Setup Timer 2 to generate a 3ms interrupt.
//
void InitialiseTimer2()
{
    TIM2_PSCR = 0x00;       //  Prescaler = 1.
    TIM2_ARRH = 0xc3;       //  High byte of 50,000.
    TIM2_ARRL = 0x50;       //  Low byte of 50,000.
    TIM2_IER_UIE = 1;       //  Enable the update interrupts.
    TIM2_CR1_CEN = 1;       //  Finally enable the timer.
}

//--------------------------------------------------------------------------------
//
//  Initialise port D.
//
void InitialisePortD()
{
    //
    //  Configure Port D to allow some output of debugging signals.
    //
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR = 0xff;          //  All pins are outputs.
    PD_CR1 = 0xff;
    PD_CR1 = 0xff;
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
void main()
{
    unsigned int frequency;

    __disable_interrupt();
    InitialiseSystemClock();
    InitialiseTimer1();
    InitialiseTimer2();
    InitialisePortD();
    __enable_interrupt();

    while (1)
    {
        frequency = MeasureFrequency();
        BitBangByte((unsigned char) (frequency & 0xff));
        BitBangByte((unsigned char) ((frequency >> 8) & 0xff));
    }
}