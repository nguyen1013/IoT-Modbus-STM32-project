/*
 * one-wire-dht22.c
 *
 *  Created on: 22.1.2026
 *      Author: nguyen
 */

#include <one-wire-dht22.h>


/* === DHT22 Driver === */
void DHT22_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    /* --- Set PB3 as output --- */
    GPIOB->MODER &= ~(3 << (3 * 2));   // clear mode bits
    GPIOB->MODER |=  (1 << (3 * 2));   // PB3 = output

    /* --- Pull line LOW for start signal --- */
    GPIOB->ODR &= ~PB3_PIN;
    delay_ms(18);

    /* --- Release line: PB3 as input --- */
    GPIOB->MODER &= ~(3 << (3 * 2));   // PB3 = input

    /* --- Wait ~20 µs for sensor response --- */
    delay_10us();
    delay_10us();
}


uint8_t DHT22_CheckResponse(void) {
	uint32_t timeout = 0;
	// Wait for sensor to pull line LOW (~80 us)
	while ((GPIOB->IDR & PB3_PIN)) {
		if (++timeout > 1000)
			return 0; // timeout
		delay_10us();
	}

	// Wait for sensor to pull line HIGH (~80 us)
	timeout = 0;
	while (!(GPIOB->IDR & PB3_PIN)) {
		if (++timeout > 1000)
			return 0;
		delay_10us();
	}

	return 1;
}

uint8_t DHT22_ReadBit(void) {
	uint8_t count = 0;

	// Wait for line to go HIGH (after 50 µs LOW)
	while (!(GPIOB->IDR & PB3_PIN))
		;

	// Measure how long it stays HIGH
	while (GPIOB->IDR & PB3_PIN) {
		delay_10us();
		count++;
		if (count > 10)
			break; // safety timeout (~100 µs)
	}

	// If HIGH lasted > ~4 * 10 µs = 40 µs → it's a '1'
	if (count > 4)
		return 1;
	else
		return 0;
}

uint8_t DHT22_ReadByte(void) {
	uint8_t i, result = 0;
	for (i = 0; i < 8; i++) {
		uint32_t timeout = 0;

		// wait for start LOW (sensor starts bit with LOW)
		while (GPIOB->IDR & PB3_PIN) {
			if (++timeout > 1000)
				return 0; // timeout
			delay_10us();
		}

		// wait for HIGH (start of bit's high pulse)
		timeout = 0;
		while (!(GPIOB->IDR & PB3_PIN)) {
			if (++timeout > 1000)
				return 0;
			delay_10us();
		}

		result <<= 1;
		if (DHT22_ReadBit())
			result |= 1;
	}
	return result;
}

/*
 * Read DHT22 and return humidity ×100 and temperature ×100
 * Returns pointer to static int32_t array:
 *   result[0] = humidity ×100 (always ≥0)
 *   result[1] = temperature ×100 (signed)
 *   result[0] = result[1] = 0 on read error
 */
int32_t* read_DHT22_value_time_100(void)
{
    __disable_irq();

    static int32_t result[2];   // result[0] = humidity×100, result[1] = temp×100

    uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2, checksum;
    uint16_t raw_humidity, raw_temperature;

    DHT22_init();

    if (!DHT22_CheckResponse()) {
        result[0] = 0;
        result[1] = 0;
        __enable_irq();
        return result;
    }

    Rh_byte1   = DHT22_ReadByte();
    Rh_byte2   = DHT22_ReadByte();
    Temp_byte1 = DHT22_ReadByte();
    Temp_byte2 = DHT22_ReadByte();
    checksum   = DHT22_ReadByte();

    if (((Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2) & 0xFF) != checksum) {
        result[0] = 0;
        result[1] = 0;
        __enable_irq();
        return result;
    }

    raw_humidity    = (Rh_byte1 << 8) | Rh_byte2;      // ×10
    raw_temperature = (Temp_byte1 << 8) | Temp_byte2;  // ×10

    // Humidity ×100
    result[0] = raw_humidity * 10;

    // Temperature ×100 (signed)
    if (raw_temperature & 0x8000) {
        raw_temperature &= 0x7FFF;
        result[1] = -(raw_temperature * 10);
    } else {
        result[1] = raw_temperature * 10;
    }

    __enable_irq();
    return result;
}
