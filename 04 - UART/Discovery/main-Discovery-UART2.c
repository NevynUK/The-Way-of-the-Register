//
//  This program shows how you can output a message on the UART on
//	the STM8S microcontroller.
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
//  Setup the UART to run at 115200 baud, no parity, one stop bit, 8 data bits.
//
//  Important: This relies upon the systemk clock being set to run at 16 MHz.
//
void InitialiseUART()
{
    //
    //  Clear the Idle Line Detected bit in the status rerister by a read
    //  to the UART2_SR register followed by a Read to the UART2_DR register.
    //
    unsigned char tmp = UART2_SR;
    tmp = UART2_DR;
    //
    //  Reset the UART registers to the reset values.
    //
    UART2_CR1 = 0;
    UART2_CR2 = 0;
    UART2_CR4 = 0;
    UART2_CR3 = 0;
    UART2_GTR = 0;
    UART2_PSCR = 0;
    //
    //  Now setup the port to 115200,n,8,1.
    //
    UART2_CR1_M = 0;        //  8 Data bits.
    UART2_CR1_PCEN = 0;     //  Disable parity.
    UART2_CR3_STOP = 0;     //  1 stop bit.
    UART2_BRR2 = 0x0a;      //  Set the baud rate registers to 115200 baud
    UART2_BRR1 = 0x08;      //  based upon a 16 MHz system clock.
    //
    //  Disable the transmitter and receiver.
    //
    UART2_CR2_TEN = 0;      //  Disable transmit.
    UART2_CR2_REN = 0;      //  Disable receive.
    //
    //  Set the clock polarity, lock phase and last bit clock pulse.
    //
    UART2_CR3_CPOL = 1;
    UART2_CR3_CPHA = 1;
    UART2_CR3_LBCL = 1;
    //
    //  Turn on the UART transmit, receive and the UART clock.
    //
    UART2_CR2_TEN = 1;
    UART2_CR2_REN = 1;
    UART2_CR3_CKEN = 1;
}

//
//  Send the message in the string to UART2.
//
void UARTPrintF(char *message)
{
    char *ch = message;
    while (*ch)
    {
        UART2_DR = (unsigned char) *ch;     //  Put the next character into the data transmission register.
        while (UART2_SR_TXE == 0);          //  Wait for transmission to complete.
        ch++;                               //  Grab the next character.
    }
}

int main( void )
{
    __disable_interrupt();
    InitialiseSystemClock();
    InitialiseUART();
    __enable_interrupt();
    while (1)
    {
        UARTPrintF("Hello from my microcontroller....\n\r");
		for (long counter = 0; counter < 250000; counter++);
    }
}
