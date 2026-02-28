#include "power_measurement.h"

#define ADC_REF_MV        3300
#define ADC_MAX           4095
#define SUPPLY_VOLTAGE     12
#define SQRT2_INV_MILLI    707   // 0.707 ≈ 707/1000

int32_t measure_power_mw(void)
{
    uint16_t adc = ADC_read_channel(1);

    int32_t vout_mv = ((int32_t)adc * ADC_REF_MV) / ADC_MAX;

    /* P(mW) = 12 * Vo / sqrt(2) */
    int32_t power_mw = (SUPPLY_VOLTAGE * vout_mv) * SQRT2_INV_MILLI/1000;

    return power_mw;
}
