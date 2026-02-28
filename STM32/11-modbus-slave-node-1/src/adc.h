#ifndef ADC_H_
#define ADC_H_

#include "stm32l1xx.h"

void ADC_init(void);
void ADC1_IRQHandler(void);
uint16_t ADC_read_channel(uint8_t ch);

#endif /* ADC_H_ */
