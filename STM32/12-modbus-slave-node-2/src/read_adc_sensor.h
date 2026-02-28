#ifndef READ_ADC_SENSOR_H_
#define READ_ADC_SENSOR_H_


#include "utility.h"
#include "./sensors/ac_measurement.h"
#include "./sensors/power_measurement.h"

int32_t read_ac_voltage_mv_median(void);
int32_t read_power_mw_median(void);


#endif /* READ_ADC_SENSOR_H_ */
