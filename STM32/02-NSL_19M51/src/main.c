/*This code tests NUCLEO-L152RE board transmitter UART communication by using
9600 BAUD and float print with sprintf
*/

/* Includes */
#include <stddef.h>
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "timer.h"
#include "usart.h"

/* Private typedef */
/* Private define  */
#define MAX_LUX 1000   // adjust based on calibration

/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */
void read_and_print_NLS19M51(void);
/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  /* Configure the system clock to 32 MHz and update SystemCoreClock */
  SetSysClock();
  SystemCoreClockUpdate();
  USART2_Init();

  //set up pin PA0 and PA1 for analog input
  RCC->AHBENR|=1;				//enable GPIOA clock
  GPIOA->MODER|=0x3;			//PA0 analog (A0)
  GPIOA->MODER|=0xC;			//PA1 analog (A1)

  //setup ADC1. p272
  RCC->APB2ENR|=0x00000200;		//enable ADC1 clock
  ADC1->CR2=0;					//bit 1=0: Single conversion mode
  ADC1->SMPR3=7;				//384 cycles sampling time for channel 0 (longest)
  ADC1->CR1&=~0x03000000;		//resolution 12-bit

  /* Infinite loop */
  while (1)
  {
	  read_and_print_NLS19M51();
	  delay_ms(1000);
  }
  return 0;
}

void read_and_print_NLS19M51(void)
{
    char buf[96];
    uint16_t adc_result = 0;
    uint32_t v_adc_mv   = 0;
    uint32_t r_ldr      = 0;
    float    lux_f      = 0.0f;
    uint32_t lux        = 0;

    /* Select ADC channel 1 (PA1) */
    ADC1->SQR5 = 1;

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Wait for EOC */
    while (!(ADC1->SR & ADC_SR_EOC));

    /* Read ADC result */
    adc_result = ADC1->DR;

    /* ADC to millivolts (Vref = 3.3V) */
    v_adc_mv = (adc_result * 3300UL) / 4095UL;

    if (v_adc_mv <= 10 || v_adc_mv >= 3290)
    {
        r_ldr = 0;
        lux   = 0;
    }
    else
    {
        /* LDR resistance (10k fixed resistor) */
        r_ldr = (3300UL - v_adc_mv) * 10000UL / v_adc_mv;

        /* ---- Log-curve compensation ----
           lux = (A / R)^(1/gamma)
           after calibration, we select A = 130000, gamma = 0.7
        */
        float gamma = 0.7f;
        if (r_ldr>5000) gamma = 0.7f;
        lux_f = powf((float)(130000/r_ldr), 1/gamma);

        /* Clamp and convert to integer */
        if (lux_f < 1.0f)     lux_f = 1.0f;
        if (lux_f > 2000.0f)  lux_f = 2000.0f;

        lux = (uint32_t)(lux_f + 0.5f);
    }

    sprintf(buf,
            "V_LDR=%dmV  R_LDR=%dohm  Lux=%d",
            (unsigned long)(3300 - v_adc_mv),
            (unsigned long)r_ldr,
            (unsigned long)lux);

    USART2_WriteString(buf);

    USART2_write('\r');
    USART2_write('\n');

    /* Disable ADC */
    ADC1->CR2 &= ~ADC_CR2_ADON;
}
