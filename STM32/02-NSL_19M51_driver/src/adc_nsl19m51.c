/*
 * adc_nsl19m51.c
 *
 *  Created on: 22.1.2026
 *      Author: nguyen
 */

#include "adc_nsl19m51.h"
#include "stm32l1xx.h"
#include <math.h>


/* ---------------------------------------------------------
 *  ADC1_init()
 *  Configure GPIOA PA1 + ADC1 once at startup
 * --------------------------------------------------------- */
void ADC1_init(void)
{
    /* Enable GPIOA clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* PA1 → analog mode */
    GPIOA->MODER |= (3U << (1 * 2));   // MODER1 = 11 (analog)

    /* Enable ADC1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ADC configuration */
    ADC1->CR2 = 0;                     // Single conversion mode
    ADC1->SMPR3 = 7;                   // Longest sampling time
    ADC1->CR1 &= ~0x03000000;          // 12‑bit resolution

    /* Select ADC channel 1 (PA1) */
    ADC1->SQR5 = 1;
}


/* ---------------------------------------------------------
 *  read_NLS19M51_time_100()
 *  Perform one ADC conversion and return lux × 100
 * --------------------------------------------------------- */
uint32_t read_NLS19M51_time_100(void)
{
    uint16_t adc_result = 0;
    uint32_t v_adc_mv   = 0;
    uint32_t r_ldr      = 0;
    float    lux_f      = 0.0f;
    uint32_t lux100     = 0;

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Wait for EOC */
    while (!(ADC1->SR & ADC_SR_EOC));

    /* Read ADC result */
    adc_result = ADC1->DR;

    /* Disable ADC */
    ADC1->CR2 &= ~ADC_CR2_ADON;

    /* Convert ADC → millivolts */
    v_adc_mv = (adc_result * 3300UL) / 4095UL;

    if (v_adc_mv <= 10 || v_adc_mv >= 3290)
    {
        return 0;
    }

    /* LDR resistance (10k fixed resistor) */
    r_ldr = (3300UL - v_adc_mv) * 10000UL / v_adc_mv;

    /* ---- Log-curve compensation ----
       lux = (A / R)^(1/gamma)
       after calibration, we select A = 130000, gamma = 0.7
    */
    float gamma = 0.7f;
    lux_f = powf((float)(130000 / r_ldr), 1.0f / gamma);

    /* Clamp */
    if (lux_f < 1.0f)     lux_f = 1.0f;
    if (lux_f > 2000.0f)  lux_f = 2000.0f;

    /* Convert to lux × 100 */
    lux100 = (uint32_t)(lux_f * 100.0f + 0.5f);

    /* UART print of LDR resistance */
    {
        char buf[48];
        sprintf(buf, "R_LDR=%d ohm --- ", r_ldr);
        USART2_WriteString(buf);
    }

    return lux100;
}
