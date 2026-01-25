/*
 * adc_nsl19m51.h
 * ADC in pin PA1
 *
 *  Created on: 22.1.2026
 *      Author: nguyen
 */

#ifndef ADC_NSL19M51_H_
#define ADC_NSL19M51_H_

#include "stm32l1xx.h"
#include <stdlib.h>
#include <math.h>
#include "usart.h"

void ADC1_init(void);
uint32_t read_NLS19M51_time_100(void);

#endif /* ADC_NSL19M51_H_ */
