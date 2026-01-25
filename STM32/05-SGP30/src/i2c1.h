#ifndef I2C1_H_
#define I2C1_H_

#include "stm32l1xx.h"


void I2C1_init(void);
void I2C1_Write(uint8_t address, uint8_t command, int n, uint8_t* data);
void I2C1_ByteWrite(uint8_t address, uint8_t command);
void I2C1_Read(uint8_t address, uint8_t command, int n, uint8_t* data);
void I2C1_BusRecover(void);

#endif /* I2C1_H_ */
