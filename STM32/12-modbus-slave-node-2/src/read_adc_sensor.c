#include <stdint.h>
#include <stdio.h>
#include "utility.h"
#include "read_adc_sensor.h"
#include "usart2.h"

/* -------- AC voltage (0–24V RMS) -------- */
int32_t read_ac_voltage_mv_median(void) {
	int32_t samples[SAMPLE_COUNT];

	/* Collect samples */
	for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
		samples[i] = measure_ac_voltage_mv();

	bubble_sort(samples, SAMPLE_COUNT);

	int32_t min = samples[0];
	int32_t max = samples[SAMPLE_COUNT - 1];
	int32_t median = samples[SAMPLE_COUNT / 2];

	/* Stability check: max variation = 1.00V */
	if ((max - min) > 1000) {
		USART2_WriteString("AC voltage not stable\n\r\n\r");
		return -9990;
	}

	/* Valid range: 0–24V RMS */
	if (median < 0 || median > 24000) {
		USART2_WriteString("AC voltage out of range\n\r\n\r");
		return -9990;
	}

	char buf[50];
	snprintf(buf, sizeof(buf), "AC Voltage = %d mV\n\r\n\r", (int) median);

	USART2_WriteString(buf);

	return median;
}

/* -------- Power Consumption (0–24W) -------- */
int32_t read_power_mw_median(void) {
	int32_t samples[SAMPLE_COUNT];

	/* Collect samples */
	for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
		samples[i] = measure_power_mw();

	bubble_sort(samples, SAMPLE_COUNT);

	int32_t min = samples[0];
	int32_t max = samples[SAMPLE_COUNT - 1];
	int32_t median = samples[SAMPLE_COUNT / 2];

	/* Stability check: max variation = 2.0W (2000 mW) */
	if ((max - min) > 2000) {
		USART2_WriteString("Power reading not stable\n\r\n\r");
		return -9990;
	}

	/* Valid range: 0–24W */
	if (median < 0 || median > 24000) {
		USART2_WriteString("Power out of range\n\r\n\r");
		return -9990;
	}

	char buf[50];
	snprintf(buf, sizeof(buf), "Power = %d mW\n\r\n\r", (int) median);

	USART2_WriteString(buf);

	return median;
}

