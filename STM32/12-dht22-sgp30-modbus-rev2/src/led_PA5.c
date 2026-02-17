#include "led_PA5.h"

#include "stm32l1xx.h"

void LED_Init(void)
{
    // Set PA5 as output (MODER5 = 01)
    GPIOA->MODER &= ~(0x3 << (5 * 2));   // Clear mode bits
    GPIOA->MODER |=  (0x1 << (5 * 2));   // Set to output mode
}

void led_on(void)
{
    GPIOA->ODR |= (1 << 5);
}

void led_off(void)
{
    GPIOA->ODR &= ~(1 << 5);
}

void led_toggle(void)
{
    GPIOA->ODR ^= (1 << 5);
}
