//
//  This program shows how you can Generate a single pulse which
//  lasts for 30 uS by using Timer 2 and Port D, Pin 4. STM8S
//  microcontroller.
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
//  Timer 2 Overflow handler.
//
#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_UPD_OVF_IRQHandler(void)
{
    unsigned char data;

    data = PD_ODR_ODR4;
    PD_ODR_ODR4 = !data;            //  Toggle Port D, pin 5.
    if (data == 1)
    {
        TIM2_IER_UIE = 0;           //  Only allow the pulse to happen once.
    }
    TIM2_SR1_UIF = 0;               //  Reset the interrupt otherwise it will fire again straight away.
}

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
//  Setup the port used to signal to the outside world that a timer even has
//  been generated.
//
void SetupOutputPorts()
{
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR_DDR4 = 1;        //  Port D, pin 4 is used as a signal.
    PD_CR1_C14 = 1;         //  Port D, pin 4 is Push-Pull
    PD_CR2_C24 = 1;         //  Port D, Pin 4 is generating a pulse under 2 MHz.
}

//
//  Setup Timer 2 to generate an interrupt every 480 clock ticks (30 uS).
//
void SetupTimer2()
{
    TIM2_PSCR = 0x00;       //  Prescaler = 1.
    TIM2_ARRH = 0x01;       //  High byte of 480.
    TIM2_ARRL = 0xe0;       //  Low byte of 480.
    TIM2_IER_UIE = 1;       //  Turn on the interrupts.
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
    SetupOutputPorts();
    SetupTimer2();
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
    }
}