#ifndef HUMIDITY_H_
#define HUMIDITY_H_

#include "stm32l1xx.h"
#include <stdlib.h>

uint32_t calculate_absolute_humidity_x100(int32_t temperature_x100,
                                          uint32_t humidity_x100);
uint16_t absolute_humidity_to_q88(uint32_t AH_x100);

#endif /* HUMIDITY_H_ */
