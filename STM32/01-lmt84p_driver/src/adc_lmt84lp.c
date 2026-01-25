/*
 * adc_lmt84lp.c
 *
 *  Created on: 22.1.2026
 *      Author: nguyen
 */

#include "adc_lmt84lp.h"
#include "stm32l1xx.h"

/* ---------------------------------------------------------
 *  ADC0_init()
 *  Configure PA0 + ADC1 once at startup
 * --------------------------------------------------------- */
void ADC0_init(void) {
	/* Enable GPIOA clock */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	/* PA0 → analog mode */
	GPIOA->MODER |= (3U << (0 * 2));   // MODER0 = 11 (analog)

	/* Enable ADC1 clock */
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	/* ADC configuration */
	ADC1->CR2 = 0;                     // Single conversion mode
	ADC1->SMPR3 = 7;                   // Longest sampling time
	ADC1->CR1 &= ~0x03000000;          // 12‑bit resolution

	/* Select ADC channel 0 (PA0) */
	ADC1->SQR5 = 0;
}

/* ---------------------------------------------------------
 *  read_lmt84lp_time_100()
 *  Perform one ADC conversion and return temperature ×100
 * --------------------------------------------------------- */
int32_t read_lmt84lp_time_100(void) {
	int adc;
	int vout_mv;
	int temp_x100;

	/* Enable ADC */
	ADC1->CR2 |= ADC_CR2_ADON;

	/* Start conversion */
	ADC1->CR2 |= ADC_CR2_SWSTART;

	/* Wait for EOC */
	while (!(ADC1->SR & ADC_SR_EOC))
		;

	/* Read ADC result */
	adc = ADC1->DR;

	/* Turn ADC off */
	ADC1->CR2 &= ~ADC_CR2_ADON;

	/* Convert ADC → millivolts */
	vout_mv = (adc * 3300) / 4095;

	/*
	 LMT84LP transfer function (approx):
	 T(°C) = (1035mV - Vout) / 5.5

	 Scaled integer:
	 T×100 = (1035 - Vout) * 1000 / 55
	 */
	temp_x100 = (1035 - vout_mv) * 1000 / 55;

	/* Clamp to sensor limits */
	if (temp_x100 > 15000)
		temp_x100 = 15000;
	if (temp_x100 < -5000)
		temp_x100 = -5000;

	return temp_x100;
}
