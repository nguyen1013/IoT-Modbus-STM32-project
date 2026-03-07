#include "adc_lmt84lp.h"

/* ---------------------------------------------------------
 *  read_lmt84lp_time_100()
 *  Perform one ADC conversion and return temperature ×100
 * --------------------------------------------------------- */
int32_t read_lmt84lp_celsius_x100(void) {
	ADC_init();

    uint16_t adc = ADC_read_channel(0);

    /* Convert ADC → millivolts */
    int32_t vout_mv = (adc * 3300) / 4095;

    /*
     LMT84LP transfer function (approx):
     T(°C) = 41250/217 - 40*Vout_mV/217

     Scaled integer:
     T×100 = 412500/217 - 400*Vout_mV/217
    */
    int32_t temp_x100 = 4125000/217 - 4000*vout_mv/217;

    return temp_x100;
}
