//
//  This program shows how you can use the ADC on the STM8S
//  microcontroller to change the PWM output.
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
//  Set up Timer 1, channel 3 to output PWM signal.
//
void SetupTimer1()
{
    TIM1_ARRH = 0x03;       //  Reload counter = 1023 (10 bits)
    TIM1_ARRL = 0xff;
    TIM1_PSCRH = 0;         //  Prescalar = 0 (i.e. 1)
    TIM1_PSCRL = 0;
    TIM1_CR1_DIR = 1;       //  Down counter.
    TIM1_CR1_CMS = 0;       //  Edge aligned counter.
    TIM1_RCR = 0;           //  Repetition count.
    TIM1_CCMR4_OC4M = 7;    //  Set up to use PWM mode 2.
    TIM1_CCER2_CC4E = 1;    //  Output is enabled.
    TIM1_CCER2_CC4P = 0;    //  Active is defined as high.
    TIM1_CCR4H = 0x03;      //  Start with the PWM signal off.
    TIM1_CCR4L = 0xff;
    TIM1_BKR_MOE = 1;       //  Enable the main output.
    TIM1_CR1_CEN = 1;
}

//--------------------------------------------------------------------------------
//
//  Timer 2 Overflow handler.
//
#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_UPD_OVF_IRQHandler(void)
{
    PD_ODR_ODR5 = !PD_ODR_ODR5;        //  Indicate that the ADC has completed.

    ADC_CR1_ADON = 1;       //  Second write starts the conversion.

    TIM2_SR1_UIF = 0;       //  Reset the interrupt otherwise it will fire again straight away.
}

//--------------------------------------------------------------------------------
//
//  Setup Timer 2 to generate an interrupt every 1/10th second based on a
//  16MHz clock.
//
void SetupTimer2()
{
    TIM2_PSCR = 0x05;       //  Prescaler = 32.
    TIM2_ARRH = 0xc3;       //  High byte of 50,000.
    TIM2_ARRL = 0x50;       //  Low byte of 50,000.
    TIM2_IER_UIE = 1;       //  Enable the update interrupts.
    TIM2_CR1_CEN = 1;       //  Finally enable the timer.
}

//--------------------------------------------------------------------------------
//
//  ADC Conversion completed interrupt handler.
//
#pragma vector = ADC1_EOC_vector
__interrupt void ADC1_EOC_IRQHandler()
{
    unsigned char low, high;
    int reading;

    ADC_CR1_ADON = 0;       //  Disable the ADC.
    TIM1_CR1_CEN = 0;       //  Disable Timer 1.
    ADC_CSR_EOC = 0;		// 	Indicate that ADC conversion is complete.

    low = ADC_DRL;			//	Extract the ADC reading.
    high = ADC_DRH;
    reading = 1023 - ((high * 256) + low);
    //
    //	Calculate the values for the capture compare register and restart Timer 1.
    //
    low = reading & 0xff;
    high = (reading >> 8) & 0xff;
    TIM1_CCR4H = high;      //  Reset the PWM counters.
    TIM1_CCR4L = low;
    TIM1_CR1_CEN = 1;       //  Restart Timer 1.

    PD_ODR_ODR4 = !PD_ODR_ODR4;     //  Indicate we have processed an ADC interrupt.
}

//--------------------------------------------------------------------------------
//
//  Setup the ADC to perform a single conversion and then generate an interrupt.
//
void SetupADC()
{
    ADC_CR1_ADON = 1;       //  Turn ADC on, note a second set is required to start the conversion.

#if defined PROTOMODULE
    ADC_CSR_CH = 0x03;
#else
    ADC_CSR_CH = 0x04;      //  ADC on AIN4 only.
#endif

    ADC_CR3_DBUF = 0;
    ADC_CR2_ALIGN = 1;      //  Data is right aligned.
    ADC_CSR_EOCIE = 1;      //  Enable the interrupt after conversion completed.
}

//--------------------------------------------------------------------------------
//
//  Now set up the output ports.
//
//  Setup the port used to signal to the outside world that a timer event has
//  been generated.
//
void SetupOutputPorts()
{
    PD_ODR = 0;             //  All pins are turned off.
    //
    //  PD5 indicates when the ADC is triggered.
    //
    PD_DDR_DDR5 = 1;
    PD_CR1_C15 = 1;
    PD_CR2_C25 = 1;
    //
    //  PD4 indicated when the ADC has completed.
    //
    PD_DDR_DDR4 = 1;
    PD_CR1_C14 = 1;
    PD_CR2_C24 = 1;
}

//--------------------------------------------------------------------------------
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
    SetupTimer2();
    SetupOutputPorts();
    SetupADC();
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
    }
}