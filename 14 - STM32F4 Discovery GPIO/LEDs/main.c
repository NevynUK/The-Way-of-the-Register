#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

int main()
{
	int pin = 13;
	uint32_t mode = GPIO_Mode_OUT << (pin * 2);
	uint32_t speed = GPIO_Speed_100MHz << (pin * 2);
	uint32_t type = GPIO_OType_PP << pin;
	uint32_t pullup = GPIO_PuPd_NOPULL << (pin * 2);

	RCC->AHB1ENR |= RCC_AHB1Periph_GPIOD;
    GPIOD->MODER |= mode;
    GPIOD->OSPEEDR |= speed;
    GPIOD->OTYPER |= type;
    GPIOD->PUPDR |= pullup;

	while (1)
	{
		GPIOD->BSRRL = (1 << pin);
		int index;
		for (index = 0; index < 500000; index++);
		GPIOD->BSRRH = (1 << pin);
		for (index = 0; index < 500000; index++);
	}
}