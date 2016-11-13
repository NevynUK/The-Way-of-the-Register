#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "system_stm32f4xx.h"

int main()
{
	SystemInit();
	SystemCoreClockUpdate();
    //
    //  Initialise the peripheral clock.
    //
    RCC->AHB1ENR |= RCC_AHB1Periph_GPIOD;
    //
    //  Initialise the GPIO port.
    //
    GPIOD->MODER |= GPIO_Mode_OUT;
    GPIOD->OSPEEDR |= GPIO_Speed_100MHz;
    GPIOD->OTYPER |= GPIO_OType_PP;
    GPIOD->PUPDR |= GPIO_PuPd_NOPULL;
    //
    //  Toggle Port D, pin 0 indefinitely.
    //
    while (1)
    {
        GPIOD->BSRRL = GPIO_Pin_0;
        GPIOD->BSRRH = GPIO_Pin_0;
    }
}
