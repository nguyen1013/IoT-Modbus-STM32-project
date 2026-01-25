/*
 * adc_lmt84lp.h
 * ADC in pin PA0
 *
 *  Created on: 22.1.2026
 *      Author: nguyen
 */

#ifndef ADC_LMT84LP_H_
#define ADC_LMT84LP_H_

#include <stdlib.h>
#include "stm32l1xx.h"

void ADC0_init(void);
int32_t read_lmt84lp_time_100(void);

#endif /* ADC_LMT84LP_H_ */
