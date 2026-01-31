#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

/* ================= Configuration ================= */
#define MODBUS_SLAVE_ADDRESS     0x01
#define MODBUS_FC_READ_INPUT_REG 0x04

/* Exception codes */
#define MODBUS_EX_ILLEGAL_FUNC   0x01
#define MODBUS_EX_ILLEGAL_REG    0x02
#define MODBUS_EX_CRC_ERROR      0x03

/* ================= Function Prototypes ================= */

void read_modbus_frame(uint8_t *rx_buf, uint8_t length);

/* Read 7 bytes from USART1 into buffer */
void read_7_bytes_from_usartx(char *received_frame);

/* Respond to Modbus master with sensor value */
void respond_frame(uint8_t slave_addr, int32_t sensor_value);

/* Debug: write string to USART2 */
void write_debug_msg(const char *str, int32_t maxchars);

/* Debug: write Modbus frame to USART2 */
void write_debug_frame(const uint8_t *frame, uint8_t length);

/* CRC16 computation */
unsigned short int CRC16 (char *nData, unsigned short int wLength);

void LED_Init(void);

#endif /* MODBUS_H */
