//
//  This program shows how you can toggle a GPIO pin on the STM8S
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

int main()
{
    //
    //  Initialise Port D.
    //
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR_DDR3 = 1;        //  Port D, bit 4 is output.
    PD_CR1_C13 = 1;         //  Pin is set to Push-Pull mode.
    PD_CR2_C23 = 1;         //  Pin can run upto 10 MHz.
    //
    //  Now lets toggle to IO line.
    //
    while (1)
    {
        PD_ODR_ODR3 = 1;    // Turn Port D, Pin 4 on.
        for (int index = 0; index < 10000; index++);
        PD_ODR_ODR3 = 0;    // Turn Port D, Pin 4 off.
        for (int index = 0; index < 10000; index++);
    }
}
