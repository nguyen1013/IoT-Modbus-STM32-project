#ifndef READ_DHT22_SGP30_H_
#define READ_DHT22_SGP30_H_

#include "stm32l1xx.h"
#include <stdint.h>
#include "./sensors/sgp30.h"

/* ================= CONFIG ================= */

#define SGP30_WARMUP_SECONDS          15
#define AH_UPDATE_THRESHOLD_X100      50   // 0.5 g/m³

/* ================= GLOBAL SENSOR STATE ================= */

extern int32_t  g_temperature_x100;   // Temperature ×100 (°C)
extern uint32_t g_humidity_x100;      // Relative humidity ×100 (%)
extern uint32_t g_ah_x100;            // Absolute humidity ×100 (g/m³)
extern sgp30_iaq_data_t iaq;

/* ================= FUNCTION PROTOTYPES ================= */

void sgp30_warmup(void);
void read_dht22_safe(void);
void update_absolute_humidity(void);
void print_environment(void);
void measure_and_print_iaq(void);
void sgp30_recover(void);

#endif /* READ_DHT22_SGP30_H_ */
