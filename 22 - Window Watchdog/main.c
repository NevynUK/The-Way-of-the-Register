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

int _firstTime = 1;

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
//  Setup Timer 2 to generate a 12.5ms interrupt.
//
void SetupTimer2()
{
    TIM2_PSCR = 0x02;       //  Prescaler = 4.
    TIM2_ARRH = 0xc3;       //  High byte of 50,000.
    TIM2_ARRL = 0x50;       //  Low byte of 50,000.
    TIM2_IER_UIE = 1;       //  Enable the update interrupts.
    TIM2_CR1_CEN = 1;       //  Finally enable the timer.
}

//--------------------------------------------------------------------------------
//
//  Initialise the Windows Watchdog.
//
void InitialiseWWDG()
{
    WWDG_CR = 0x5b;         //  Approx 70ms total window.
    WWDG_WR = 0x4c;         //  Approx 11.52ms window where cannot reset
    WWDG_CR_WDGA = 1;       //  Enable the watchdog.
}

#define SR_CLOCK            PD_ODR_ODR5
#define SR_DATA             PD_ODR_ODR3

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
//  Timer 2 Overflow handler.
//
#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_UPD_OVF_IRQHandler(void)
{
    if (_firstTime)
    {
        InitialiseWWDG();
        _firstTime = 0;
    }
    else
    {
        unsigned char counter = (unsigned char) WWDG_CR;
        unsigned char window = WWDG_WR;
        BitBangByte(counter & 0x7f);
        BitBangByte(window);
        WWDG_CR = 0xdb;     //  Reset the Window Watchdog counter.
        counter = (unsigned char) WWDG_CR;
        BitBangByte(counter);
    }
    PD_ODR_ODR2 = !PD_ODR_ODR2;
    TIM2_SR1_UIF = 0;       //  Reset the interrupt otherwise it will fire again straight away.
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
    PD_ODR_ODR4 = 1;
    __no_operation();
    PD_ODR_ODR4 = 0;
    SetupTimer2();
    InitialiseWWDG();
    __enable_interrupt();
    //
    //  Main program loop.
    //
    while (1)
    {
        __wait_for_interrupt();
    }
}