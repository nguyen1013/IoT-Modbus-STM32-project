#include "sgp30.h"
#include "timer.h"

/* ================= SGP30 Initialization ================= */
void sgp30_iaq_init(void)
{
    uint8_t cmd[2];

    // Split 16-bit command into MSB/LSB
    cmd[0] = (uint8_t)(SGP30_CMD_IAQ_INIT >> 8);
    cmd[1] = (uint8_t)(SGP30_CMD_IAQ_INIT & 0xFF);

    // Send command (no payload)
    I2C1_Write(SGP30_ADDR, cmd[0], 1, &cmd[1]);

    // Wait 10 ms as required by datasheet
    delay_ms(10);
}

/* ================= Set Absolute Humidity ================= */
void sgp30_set_absolute_humidity(uint16_t abs_humidity)
{
    uint8_t data[4];

    // data[0] = CMD_LSB
    data[0] = (uint8_t)(SGP30_CMD_SET_ABS_HUMIDITY & 0xFF);

    // data[1] = humidity MSB
    data[1] = (uint8_t)(abs_humidity >> 8);

    // data[2] = humidity LSB
    data[2] = (uint8_t)(abs_humidity & 0xFF);

    // data[3] = CRC over humidity bytes
    data[3] = sgp30_crc8(&data[1]);

    // Send: CMD_MSB, CMD_LSB, HUM_MSB, HUM_LSB, CRC
    I2C1_Write(SGP30_ADDR, (uint8_t)(SGP30_CMD_SET_ABS_HUMIDITY >> 8), 4, data);

    delay_ms(2);
}

/* ================= Measure IAQ ================= */
void sgp30_measure_iaq(sgp30_iaq_data_t *out)
{
    uint8_t cmd[2];
    uint8_t buf[SGP30_IAQ_RESPONSE_LEN];

    // Split 16-bit command into MSB/LSB
    cmd[0] = (uint8_t)(SGP30_CMD_MEASURE_IAQ >> 8);
    cmd[1] = (uint8_t)(SGP30_CMD_MEASURE_IAQ & 0xFF);

    // Send command (no payload)
    I2C1_Write(SGP30_ADDR, cmd[0], 1, &cmd[1]);

    // Wait 12 ms for measurement (datasheet)
    delay_ms(12);

    // Read 6 bytes: eCO2(2)+CRC + TVOC(2)+CRC
    I2C1_Read(SGP30_ADDR, 0x00, SGP30_IAQ_RESPONSE_LEN, buf);

    // Validate CRC for eCO2
    if (sgp30_crc8(buf) != buf[2]) {
        out->eco2_ppm = 0xFFFF;   // error marker
        out->tvoc_ppb = 0xFFFF;
        return;
    }

    // Validate CRC for TVOC
    if (sgp30_crc8(&buf[3]) != buf[5]) {
        out->eco2_ppm = 0xFFFF;
        out->tvoc_ppb = 0xFFFF;
        return;
    }

    // Extract values
    out->eco2_ppm = ((uint16_t)buf[0] << 8) | buf[1];
    out->tvoc_ppb = ((uint16_t)buf[3] << 8) | buf[4];
}

/* ================= CRC8 Calculation ================= */
uint8_t sgp30_crc8(const uint8_t *data)
{
    uint8_t crc = 0xFF;
    uint8_t i, b;

    for (b = 0; b < 2; b++)   // SGP30 CRC is always over 2 bytes
    {
        crc ^= data[b];
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }

    return crc;
}
