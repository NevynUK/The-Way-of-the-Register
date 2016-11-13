#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

int main()
{
    //
    //  Initialise the peripheral clock.
    //
    RCC->AHB1ENR |= RCC_AHB1Periph_GPIOD;
    //
    //  Initilaise the GPIO port.
    //
    GPIOD->MODER |= (GPIO_Mode_OUT << 26);
    GPIOD->OSPEEDR |= (GPIO_Speed_100MHz << 26);
    GPIOD->OTYPER |= (GPIO_OType_PP << 13);
    GPIOD->PUPDR |= (GPIO_PuPd_NOPULL << 26);
    //
    //  Toggle Port D, pin 0 indefinetly.
    //
    GPIOD->ODR = 0;
    while (1)
    {
        GPIOD->BSRRL = GPIO_Pin_13;
        for (int index = 0; index < 1000; index++);
        GPIOD->BSRRH = GPIO_Pin_13;
        for (int index = 0; index < 1000; index++);
    }
//    GPIOD->ODR = 0x0000ffff;    // GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
}