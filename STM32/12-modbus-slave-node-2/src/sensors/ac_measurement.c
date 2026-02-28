#include "ac_measurement.h"

#define ADC_REF_MV        3300
#define ADC_MAX           4095
#define GAIN              12
#define OFFSET_MV         1200
#define SQRT2_MILLI   	707   // 0.707 ≈ 707/1000

int32_t measure_ac_voltage_mv(void) {
	uint16_t adc = ADC_read_channel(0);

	/* ADC → mV */
	int32_t vout_mv = ((int32_t) adc * ADC_REF_MV) / ADC_MAX;

	/* Apply gain + offset */
	int32_t temp_mv = vout_mv * GAIN + OFFSET_MV;

	/* Divide by sqrt(2) using fixed-point */
	int32_t Vrms_mv = temp_mv * SQRT2_MILLI / 1000;

	return Vrms_mv;
}
