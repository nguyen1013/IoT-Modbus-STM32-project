#include "read_dht22_sgp30.h"

#include <stdio.h>
#include <stdlib.h>

#include "usart2.h"
#include "i2c1.h"
#include "./sensors/one-wire-dht22.h"
#include "./sensors/humidity.h"

/* ================= GLOBAL STATE ================= */

int32_t g_temperature_x100 = 0;
uint32_t g_humidity_x100 = 0;
uint32_t g_ah_x100 = 0xFFFFFFFF;

sgp30_iaq_data_t iaq;

static char buf[64];

/* ================= SGP30 WARMUP ================= */

void sgp30_warmup(void) {
	delay_ms(1000);
	sgp30_iaq_init();

	USART2_WriteString("SGP30 warm-up starting...\r\n");

	for (int i = 0; i < SGP30_WARMUP_SECONDS; i++) {
		sgp30_iaq_data_t dummy;
		sgp30_measure_iaq(&dummy);

		sprintf(buf, "Warm-up: %d/%d s\r\n", i + 1, SGP30_WARMUP_SECONDS);
		USART2_WriteString(buf);

		delay_ms(1000);
	}

	USART2_WriteString("SGP30 warm-up complete\r\n\r\n");
}

/* ================= DHT22 ================= */

void read_dht22_safe(void) {
	int32_t *dht = read_DHT22_value_time_100();

	if (dht[0] == 0 && dht[1] == 0) {
		USART2_WriteString("DHT22 read error\r\n\r\n");
		return;
	}

	g_humidity_x100 = dht[0];
	g_temperature_x100 = dht[1];
}

/* ================= ABSOLUTE HUMIDITY ================= */
void update_absolute_humidity(void) {
	uint32_t ah_x100 = calculate_absolute_humidity_x100(g_temperature_x100,
			g_humidity_x100);

	if (g_ah_x100 == 0xFFFFFFFF
			|| ah_x100 > g_ah_x100 + AH_UPDATE_THRESHOLD_X100
			|| ah_x100 + AH_UPDATE_THRESHOLD_X100 < g_ah_x100) {
		uint16_t ah_q88 = absolute_humidity_to_q88(ah_x100);
		sgp30_set_absolute_humidity(ah_q88);
		g_ah_x100 = ah_x100;
	}
}

/* ================= ENVIRONMENT PRINT ================= */

void print_environment(void) {

	/* Check for sensor error */
	if (g_temperature_x100 == -9990 || g_humidity_x100 == -9990
			|| g_ah_x100 == -9990) {
		USART2_WriteString("Environment: DHT22 read error\r\n\r\n");
		return;
	}

	/* Temperature */
	USART2_WriteString("Temp: ");
	sprintf(buf, "%d", (int) (g_temperature_x100 / 100));
	USART2_WriteString(buf);
	USART2_WriteString(".");
	sprintf(buf, "%d", (int) (g_temperature_x100 % 100));
	if ((g_temperature_x100 % 100) < 10 && (g_temperature_x100 % 100) > -10)
		USART2_WriteString("0");
	USART2_WriteString(buf);
	USART2_WriteString(" C\r\n");

	/* Humidity */
	USART2_WriteString("Humidity: ");
	sprintf(buf, "%d", (int) (g_humidity_x100 / 100));
	USART2_WriteString(buf);
	USART2_WriteString(".");
	sprintf(buf, "%d", (int) (g_humidity_x100 % 100));
	if ((g_humidity_x100 % 100) < 10)
		USART2_WriteString("0");
	USART2_WriteString(buf);
	USART2_WriteString(" %\r\n");

	/* Absolute humidity */
	USART2_WriteString("Abs Humidity: ");
	sprintf(buf, "%d", (int) (g_ah_x100 / 100));
	USART2_WriteString(buf);
	USART2_WriteString(".");
	sprintf(buf, "%d", (int) (g_ah_x100 % 100));
	if ((g_ah_x100 % 100) < 10)
		USART2_WriteString("0");
	USART2_WriteString(buf);
	USART2_WriteString(" g/m3\r\n\r\n");
}

/* ================= IAQ ================= */
void measure_and_print_iaq(void) {
	/* If DHT22 failed earlier, skip IAQ measurement */
	if (g_temperature_x100 == -9990 || g_humidity_x100 == -9990) {
		USART2_WriteString("IAQ skipped: DHT22 error\r\n\r\n");
		iaq.eco2_ppm = -9990;
		iaq.tvoc_ppb = -9990;
		return;
	}

	sgp30_measure_iaq(&iaq);

	/* SGP30 error → try recovery */
	if (iaq.eco2_ppm == 0xFFFF || iaq.tvoc_ppb == 0xFFFF) {
		USART2_WriteString("SGP30 error, recovering I2C...\r\n\r\n");

		iaq.eco2_ppm = -9990;
		iaq.tvoc_ppb = -9990;

		sgp30_recover();
		sgp30_measure_iaq(&iaq);

		/* Recovery failed → set error code */
		if (iaq.eco2_ppm == 0xFFFF || iaq.tvoc_ppb == 0xFFFF) {
			USART2_WriteString("SGP30 recovery failed\r\n\r\n");
			iaq.eco2_ppm = -9990;
			iaq.tvoc_ppb = -9990;
			return;
		}
	}

	/* Normal printing */
	sprintf(buf, "eCO2: %d ppm\r\n", (int) iaq.eco2_ppm);
	USART2_WriteString(buf);

	sprintf(buf, "TVOC: %d ppb\r\n\r\n", (int) iaq.tvoc_ppb);
	USART2_WriteString(buf);
}

/* ================= I2C RECOVERY ================= */

void sgp30_recover(void) {
	USART2_WriteString("Recovering I2C bus...\r\n");

	I2C1_BusRecover();
	I2C1_init();

	delay_ms(10);
	sgp30_iaq_init();
	delay_ms(10);
}
