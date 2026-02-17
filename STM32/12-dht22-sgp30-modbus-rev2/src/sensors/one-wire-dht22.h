/*
 * one-wire-dht22.h
 *
 *  Created on: 22.1.2026
 *      Author: nguyen
 */

#ifndef ONE_WIRE_DHT22_H_
#define ONE_WIRE_DHT22_H_

#include "stm32l1xx.h"
#include <stdlib.h>

#include "timer.h"

#define PB3_PIN   (1 << 3)   // DHT22 data pin (Arduino D3)
#define HSI_VALUE ((uint32_t)16000000)

void DHT22_init(void);
uint8_t DHT22_CheckResponse(void);
uint8_t DHT22_ReadBit(void);
uint8_t DHT22_ReadByte(void);
int32_t* read_DHT22_value_time_100(void);

#endif /* 1_WIRE_DHT22_H_ */
