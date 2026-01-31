#include "adc_nsl19m51.h"
#include <stdio.h>
#include <math.h>
#include "usart2.h"
#include "adc.h"

/* ---------------------------------------------------------
 *  read_NLS19M51_lux()
 *  Perform one ADC conversion and return luminance in lux
 * --------------------------------------------------------- */
uint16_t read_NLS19M51_lux(void)
{
    uint16_t adc_result = 0;
    uint16_t v_adc_mv   = 0;
    float    r_ldr_f    = 0.0f;
    uint16_t lux      = 0.0f;

    /* ---- Read ADC result ---- */
    adc_result = ADC_read_channel(1);

    /* ---- Convert ADC → millivolts ---- */
    v_adc_mv = (adc_result * 3300UL) / 4095UL;

    /* Guard against division by zero or near-zero */
    if (v_adc_mv < 10) v_adc_mv = 10;

    /* ---- LDR resistance calculation using float ---- */
    r_ldr_f = ((float)(3300UL - v_adc_mv) * 10000.0f) / (float)v_adc_mv;

    /* ---- Logarithmic lux calculation ----
       lux = (A / R)^(1/gamma)
       Calibration constants: A = 130000, gamma = 0.7
    */
    float gamma = 0.7f;
    if (r_ldr_f < 5000.0f) gamma = 0.65f; // after calibration with luminance > 100lux

    lux = (uint16_t) powf(130000.0f / r_ldr_f, 1.0f / gamma);

    /* Clamp lux to avoid unrealistic values */
    if (lux < 1.0f) lux = 1.0f;
//    if (lux > 2000.0f) lux = 2000.0f;

    /* ---- Debug prints ---- */
    {
        char buf[64];
        sprintf(buf, "R_LDR=%d ohm --- ", (int) r_ldr_f);
        USART2_WriteString(buf);
    }
    {
        char buf[64];
        sprintf(buf, "adc=%d --- ", adc_result);
        USART2_WriteString(buf);
    }
    {
        char buf[64];
        sprintf(buf, "lux=%d --- ", (int) lux);
        USART2_WriteString(buf);
    }

    return lux;
}
