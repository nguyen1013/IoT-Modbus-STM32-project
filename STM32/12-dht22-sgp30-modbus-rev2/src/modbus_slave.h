#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

#define MODBUS_SLAVE_ADDRESS     0x01
#define MODBUS_FC_READ_INPUT_REG 0x04

/* Exception codes */
#define MODBUS_EX_ILLEGAL_FUNC   0x01
#define MODBUS_EX_ILLEGAL_REG    0x02
#define MODBUS_EX_CRC_ERROR      0x03

void modbus_slave_init(void);

void respond_frame(uint8_t slave_addr, int32_t sensor_value);

/* Debug */
void write_debug_msg(const char *str, int32_t maxchars);
void write_debug_frame(const volatile uint8_t *frame, uint8_t length);

/* CRC16 computation */
unsigned short int CRC16 (char *nData, unsigned short int wLength);

#endif /* MODBUS_H */
