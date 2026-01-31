#include "adc_lmt84lp.h"

/* ---------------------------------------------------------
 *  read_lmt84lp_time_100()
 *  Perform one ADC conversion and return temperature ×100
 * --------------------------------------------------------- */
int32_t read_lmt84lp_celsius_x10(void) {
	ADC_init();

    uint16_t adc = ADC_read_channel(0);

    /* Convert ADC → millivolts */
    int32_t vout_mv = (adc * 3300) / 4095;

    /*
     LMT84LP transfer function (approx):
     T(°C) = (1035mV - Vout) / 5.5

     Scaled integer:
     T×10 = (1035 - Vout) * 100 / 55
    */
    int32_t temp_x10 = (1035 - vout_mv) * 100 / 55;

    /* Clamp to sensor limits */
    if (temp_x10 > 1500)
        temp_x10 = 1500;
    if (temp_x10 < -500)
        temp_x10 = -500;

    return temp_x10;
}
