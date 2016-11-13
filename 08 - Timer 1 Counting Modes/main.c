#include <iostm8s103f3.h>
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
//--------------------------------------------------------------------------------
//
//  Timer 1 Overflow handler.
//
#pragma vector = TIM1_OVR_UIF_vector
__interrupt void TIM1_UPD_OVF_IRQHandler(void)
{
    PC_ODR_ODR4 = 0;
    TIM1_CR1_CEN = 0;               //  Stop Timer 1.
    TIM1_SR1_UIF = 0;               //  Reset the interrupt otherwise it will fire again straight away.
}

//--------------------------------------------------------------------------------
//
//  Set up Timer 1, channel 3 to output a single pulse lasting 240 uS.
//
void SetupTimer1()
{
    TIM1_ARRH = 0x02;       //  Reload counter = 960
    TIM1_ARRL = 0x00;
    TIM1_PSCRH = 0xf4;      //  Prescalar = 0 (i.e. 1)
    TIM1_PSCRL = 0x24;
    TIM1_IER_UIE = 1;       //  Turn interrupts on.
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
void main()
{
    __disable_interrupt();
    InitialiseSystemClock();
    SetupTimer1();
    __enable_interrupt();
    //
    //  Setup the output pin.
    //
    PC_DDR_DDR4 = 1;    	//  Output pin.
    PC_CR1_C14 = 1;         //  Push-pull output.
    PC_CR2_C24 = 0;         //  Low speed, this interrupt is 2s long.
    PC_ODR_ODR4 = 1;
    //
    //  Force Timer 1 to update without generating an interrupt.
    //  This is necessary to makes sure we start off with the correct
    //  number of PWM pulses for the first instance only.
    //
    TIM1_CR1_URS = 1;
    TIM1_EGR_UG = 1;
    //
    //  Enable Timer 1
    //
    TIM1_CR1_CEN = 1;           //  Start Timer 1.
    while (1)
    {
        __wait_for_interrupt();
    }
}