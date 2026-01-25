#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>

#include "usart.h"
#include "one-wire-dht22.h"
#include "sgp30.h"
#include "i2c1.h"
#include "humidity.h"

/* ================= CONFIG ================= */

#define SGP30_WARMUP_SECONDS          15
#define AH_UPDATE_THRESHOLD_X100      50   // 0.5 g/m³

/* ================= STATE ================= */

static int32_t g_temperature_x100 = 0;
static uint32_t g_humidity_x100 = 0;
static uint32_t g_ah_x100 = 0xFFFFFFFF;

static char buf[64];

static void sgp30_warmup(void);
static void read_dht22_safe(void);
static void update_absolute_humidity(void);
static void print_environment(void);
static void measure_and_print_iaq(void);
static void sgp30_recover(void);

/* ================= MAIN ================= */

int main(void) {
	SetSysClock();
	SystemCoreClockUpdate();

	I2C1_init();
	USART2_Init();
	DHT22_init();

	sgp30_warmup();

	while (1) {
		read_dht22_safe();
		update_absolute_humidity();
		print_environment();
		measure_and_print_iaq();

		delay_ms(1000);
	}

}

/* ================= SGP30 ================= */
static void sgp30_warmup(void)
{
    delay_ms(1000);      // Power-up stabilization
    sgp30_iaq_init();

    USART2_WriteString("SGP30 warm-up starting...\r\n");

    for (int i = 0; i < SGP30_WARMUP_SECONDS; i++)
    {
        sgp30_iaq_data_t dummy;
        sgp30_measure_iaq(&dummy);

        // Print warm-up progress
        sprintf(buf, "Warm-up: %d/%d s\r\n", i + 1, SGP30_WARMUP_SECONDS);
        USART2_WriteString(buf);

        delay_ms(1000);
    }

    USART2_WriteString("SGP30 warm-up complete\r\n\r\n");
}

/* ================= DHT22 ================= */
static void read_dht22_safe(void)
{
    int32_t *dht = read_DHT22_value_time_100();

    if (dht[0] == 0 && dht[1] == 0) {
        USART2_WriteString("DHT22 read error\r\n");
        return;
    }

    g_humidity_x100    = dht[0];    // humidity ×100
    g_temperature_x100 = dht[1];    // temperature ×100 (signed)
}


/* ================= HUMIDITY ================= */
static void update_absolute_humidity(void) {
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

/* ================= PRINTING ================= */
static void print_environment(void) {
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
	USART2_WriteString(" g/m3\r\n");
}

/* ================= IAQ ================= */
static void measure_and_print_iaq(void)
{
    sgp30_iaq_data_t iaq;

    sgp30_measure_iaq(&iaq);

    if (iaq.eco2_ppm == 0xFFFF || iaq.tvoc_ppb == 0xFFFF) {

        USART2_WriteString("SGP30 error, recovering I2C...\r\n");

        sgp30_recover();

        /* Retry once */
        sgp30_measure_iaq(&iaq);

        if (iaq.eco2_ppm == 0xFFFF || iaq.tvoc_ppb == 0xFFFF) {
            USART2_WriteString("SGP30 recovery failed\r\n\r\n");
            return;
        }
    }

    USART2_WriteString("eCO2: ");
    sprintf(buf, "%d", (int)iaq.eco2_ppm);
    USART2_WriteString(buf);
    USART2_WriteString(" ppm\r\n");

    USART2_WriteString("TVOC: ");
    sprintf(buf, "%d", (int)iaq.tvoc_ppb);
    USART2_WriteString(buf);
    USART2_WriteString(" ppb\r\n\r\n");
}

static void sgp30_recover(void)
{
    USART2_WriteString("Recovering I2C bus...\r\n");

    I2C1_BusRecover();
    I2C1_init();

    delay_ms(10);
    sgp30_iaq_init();
    delay_ms(10);
}

