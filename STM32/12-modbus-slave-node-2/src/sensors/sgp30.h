#ifndef SGP30_H_
#define SGP30_H_

#include <stdint.h>
#include "I2C1.h"

/* -------------------------------------------------------------------------
 *  SGP30 I2C ADDRESS (7-bit)
 * ------------------------------------------------------------------------- */
#define SGP30_ADDR                  0x58

/* -------------------------------------------------------------------------
 *  FUNDAMENTAL COMMANDS (16-bit)
 * ------------------------------------------------------------------------- */
#define SGP30_CMD_IAQ_INIT          0x2003
#define SGP30_CMD_MEASURE_IAQ       0x2008
#define SGP30_CMD_SET_ABS_HUMIDITY  0x2061

/* -------------------------------------------------------------------------
 *  RESPONSE LENGTHS
 * ------------------------------------------------------------------------- */
#define SGP30_IAQ_RESPONSE_LEN      6   // TVOC + CRC, eCO2 + CRC
#define SGP30_HUMIDITY_PARAM_LEN    3   // 2 bytes + CRC

typedef struct {
	int32_t tvoc_ppb;
	int32_t eco2_ppm;
} sgp30_iaq_data_t;

void sgp30_iaq_init(void);
void sgp30_set_absolute_humidity(uint16_t abs_humidity);
void sgp30_measure_iaq(sgp30_iaq_data_t *out);
uint8_t sgp30_crc8(const uint8_t *data);

#endif /* SGP30_H_ */
