#include <stdint.h>
#include <stdio.h>
#include "utility.h"
#include "read_adc_sensor.h"
#include "usart2.h"


/* -------- Temperature: LMT84LP -------- */
int32_t read_lmt84lp_celsius_x100_median(void) {
    int32_t samples[SAMPLE_COUNT];

    for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
        samples[i] = read_lmt84lp_celsius_x100();

    bubble_sort(samples, SAMPLE_COUNT);

    int32_t min = samples[0];
    int32_t max = samples[SAMPLE_COUNT - 1];
    int32_t median = samples[SAMPLE_COUNT / 2];

    /* Stability check: max variation = 3.00°C */
    if ((max - min) > 300) {
        USART2_WriteString("ADC values are not stable\n\r");
        return -9990;
    }

    /* Valid range: -50.00°C to 150.00°C */
    if (median < -5000 || median > 15000) {
        USART2_WriteString("Sensor reading error\n\r");
        return -9990;
    }

    /* Separate integer and fractional parts */
    int whole = median / 100;       // integer part
    int fraction = median % 100;    // fractional part
    if (fraction < 0) fraction = -fraction;  // make fraction positive if temperature is negative

    char buf[40];
    if (fraction>9)
    {
        sprintf(buf, "Temperature = %d.%d Celsius\n\r", whole, fraction);
    }
    else
    {
    	sprintf(buf, "Temperature = %d.0%d Celsius\n\r", whole, fraction);
    }

    USART2_WriteString(buf);

    return median;
}

/* -------- Luminance: NSL19M51 -------- */
int32_t read_NLS19M51_lux_median(void) {
    int32_t samples[SAMPLE_COUNT];

    for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
        samples[i] = read_NLS19M51_lux();

    bubble_sort(samples, SAMPLE_COUNT);

    int32_t min = samples[0];
    int32_t max = samples[SAMPLE_COUNT - 1];
    int32_t median = samples[SAMPLE_COUNT / 2];

    /* Stability check: max variation = 20% */
    if (median != 0 && (max - min) > (median / 5))
    {
    	USART2_WriteString("ACD values are not stable\n\r");
        return -9990;
    }

    /* Valid range: 0 to 10000 lux */
    if (median < 0 || median > 10000)
    {
    	USART2_WriteString("Sensor reading error\n\r");
        return -9990;
    }

    return median;
}

