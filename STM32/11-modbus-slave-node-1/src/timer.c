/*
 * timer.c
 *
 *  Created on: 27 thg 11, 2025
 *      Author: nguyen
 */

#include "stm32l1xx.h"
#include "timer.h"

void delay_ms(unsigned long delay)
{
	unsigned long i=0;
	RCC->APB1ENR|=(1<<3); 	//TIM5EN: Timer 5 clock enable. p160
	TIM5->PSC=32-1; 		//32 000 000 MHz / 32 = 1 000 000 Hz. p435
	TIM5->ARR=1000-1; 		//TIM5 counter. 1 000 000 Hz / 1000 = 1000 Hz ~ 1ms. p435
	TIM5->CNT=0;			//counter start value = 0
	TIM5->CR1=1; 			//TIM5 Counter enabled. p421

	  while(i<delay)
	  {
		  while(!((TIM5->SR)&1)){} //Update interrupt flag. p427
		  i++;
		  TIM5->SR &= ~1; 	//flag cleared. p427
		  TIM5->CNT=0;	  	//counter start value = 0
	  }
	  TIM5->CR1=0; 		//TIM5 Counter disabled. p421
}


void delay_10us(void) {
	unsigned long i = 0;
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->PSC = 0;
	TIM6->ARR = 32 - 1;
	TIM6->CNT = 0;
	TIM6->CR1 = TIM_CR1_CEN;

	while (i < 5) {
		while (!(TIM6->SR & TIM_SR_UIF)) {
		}
		TIM6->SR &= ~TIM_SR_UIF;
		TIM6->CNT = 0;
		i++;
	}
	TIM6->CR1 = 0;
}

