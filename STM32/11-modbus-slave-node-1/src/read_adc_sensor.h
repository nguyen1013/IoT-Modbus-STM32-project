#ifndef READ_ADC_SENSOR_H_
#define READ_ADC_SENSOR_H_


#include "utility.h"
#include "./sensors/adc_lmt84lp.h"
#include "./sensors/adc_nsl19m51.h"

int32_t read_lmt84lp_celsius_x100_median(void);
int32_t read_NLS19M51_lux_median(void);


#endif /* READ_ADC_SENSOR_H_ */
