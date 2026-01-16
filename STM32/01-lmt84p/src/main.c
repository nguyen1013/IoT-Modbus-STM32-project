/*LMT84p
*/

/* Includes */
#include <stddef.h>
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>

#include "timer.h"
#include "usart.h"

/* Private typedef */
/* Private define  */
/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

void read_and_print_lmt84p(void);
void delay_Ms(int delay)
{
	int i=0;
	for(; delay>0;delay--)
		for(i=0;i<2460;i++); //measured with oscilloscope
}

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
    __disable_irq();         // Disable global IRQs

	  /* Configure the system clock to 32 MHz and update SystemCoreClock */
	  SetSysClock();
	  SystemCoreClockUpdate();
	  USART2_Init();
	  /* TODO - Add your application code here */

	  //set up pin PA5 for LED
	  RCC->AHBENR |= 1;				//enable GPIOA clock
	  GPIOA->MODER&=~0x00000C00;	//clear pin mode
	  GPIOA->MODER|=0x00000400;		//set pin PA5 to output model

	  //set up pin PA0 and PA1 for analog input
	  RCC->AHBENR|=1;				//enable GPIOA clock
	  GPIOA->MODER|=0x3;			//PA0 analog (A0)
	  GPIOA->MODER|=0xC;			//PA1 analog (A1)

	  //setup ADC1. p272
	  RCC->APB2ENR|=0x00000200;		//enable ADC1 clock
	  ADC1->CR2=0;					//bit 1=0: Single conversion mode
	  ADC1->SMPR3=7;				//384 cycles sampling time for channel 0 (longest)
	  ADC1->CR1&=~0x03000000;		//resolution 12-bit

	    __enable_irq();          // Enable global IRQs

  /* Infinite loop */
  while (1)
  {
	  read_and_print_lmt84p();
	  delay_ms(1000);

  }
  return 0;
}

void read_and_print_lmt84p(void)
{
    char buf[100];
    int adc;
    int vout_mv;
    int temp_x10;
    int temp_int;
    int temp_frac;

    ADC1->SQR5 = 0;              // Channel A0
    ADC1->CR2 |= 1;              // ADC ON
    ADC1->CR2 |= 0x40000000;     // Start conversion

    while (!(ADC1->SR & 2)) {}   // Wait for EOC

    adc = ADC1->DR;

    /* ADC → millivolts (12-bit, 3.3V reference) */
    vout_mv = (adc * 3300) / 4095;

    /* LMT84P reverse-linear conversion (NO floats) */
    temp_x10 = (1034-vout_mv)*10/5.5;

    /* Clamp to sensor limits */
    if (temp_x10 > 1500) temp_x10 = 1500;
    if (temp_x10 < -500) temp_x10 = -500;

    temp_int  = temp_x10 / 10;
    temp_frac = temp_x10 % 10;
    if (temp_frac < 0) temp_frac = -temp_frac;

    /* Format output */
    sprintf(buf, "ADC=%d Vout=%d Temp=%d.%d C\r\n",
            adc,vout_mv, temp_int, temp_frac);

    USART2_WriteString(buf);

    /* Optional: turn ADC off to save power */
    ADC1->CR2 &= ~ADC_CR2_ADON;
}
