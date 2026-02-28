#include "adc.h"

/* For ACD in PA0 and PA1 */
void ADC_init(void) {
    /* Enable GPIOA clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* PA0 and PA1 → analog mode */
    GPIOA->MODER |= (3U << (0 * 2));   // PA0 = ADC channel 0
    GPIOA->MODER |= (3U << (1 * 2));   // PA1 = ADC channel 1


    /* Enable ADC1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* Basic ADC configuration */
    ADC1->CR2 = 0;                     // Single conversion mode
    ADC1->SMPR3 =
        (7 << (0 * 3)) |   // channel 0
        (7 << (1 * 3));    // channel 1

    ADC1->CR1 &= ~0x03000000;          // 12‑bit resolution
}

uint16_t ADC_read_channel(uint8_t ch)
{
    /* Select channel */
    ADC1->SQR5 = ch & 0x1F;

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* ---- Dummy conversion (CRITICAL) ---- */
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    (void)ADC1->DR;   // discard

    /* ---- Real conversion ---- */
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    uint16_t result = ADC1->DR;

    /* Disable ADC */
    ADC1->CR2 &= ~ADC_CR2_ADON;

    return result;
}
