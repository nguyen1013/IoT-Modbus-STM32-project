#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>

#include "usart2.h"
#include "usart1.h"
#include "led_PA5.h"
#include "adc.h"
#include "i2c1.h"
#include "modbus_slave.h"
#include "./sensors/one-wire-dht22.h"
#include "./sensors/sgp30.h"
#include "./sensors/humidity.h"
#include "read_dht22_sgp30.h"

int32_t read_sensor(uint8_t input_address);

/************************************************************************/
/* Flags are volatile, since the flag value can change during interrupt */
/* handler. Therefore compiler can handle it correctly.                 */
/************************************************************************/
volatile char mFlag=0;
volatile uint8_t neFlag = 0;
volatile uint8_t frameFlag = 0;

/* Modbus request buffer:
   slave + func + addrH + addrL + qtyH + qtyL + crcL + crcH = 8 bytes */
volatile uint8_t request_frame[8];
volatile uint8_t rx_index = 0;
volatile uint8_t frame_ready = 0;

char buf[64];

/* ================= MAIN ================= */

int main(void) {
	__disable_irq();			//global disable IRQs, M3_Generic_User_Guide p135.

	/* Configure the system clock to 32 MHz and update SystemCoreClock */
	SetSysClock();
	SystemCoreClockUpdate();

    USART1_Init();   // Modbus RTU
    USART2_Init();   // Debug
    ADC_init();
    LED_Init();
	I2C1_init();
	DHT22_init();

	sgp30_warmup();

	modbus_slave_init();

    USART1->CR1 |= USART_CR1_RXNEIE;   // RX interrupt enable
    NVIC_EnableIRQ(USART1_IRQn);

	__enable_irq();

    while (1)
    {
        if (frameFlag)
        {
        	USART2_WriteString("Framing Error Detected");
            frameFlag = 0;
        }

        if (neFlag)
        {
        	USART2_WriteString("Noise Error Detected");
            neFlag = 0;
        }

        /* -------- Complete Modbus frame received -------- */
        if (frame_ready)
        {
        	write_debug_frame(request_frame, 8);
            uint16_t crc = CRC16((char *)request_frame, 6);

            /* CRC validation */
            if (request_frame[6] == (crc & 0xFF) &&
                request_frame[7] == ((crc >> 8) & 0xFF))
            {
            	uint8_t slave_addr = request_frame[0];

                if (slave_addr == 0x03 || slave_addr == 0x04)
                {

                    if (request_frame[1] == 0x04 &&
                        request_frame[2] == 0x00 &&
                        request_frame[3] == 0x01 &&
                        request_frame[4] == 0x00 &&
                        request_frame[5] == 0x02)
                    {
                        int32_t sensor_value = read_sensor(slave_addr);

                        if (sensor_value != -9999)
                        {
                            respond_frame(slave_addr, sensor_value);
                        }
                    }
                }
            }

            frame_ready = 0;
        }
    }
}

void USART1_IRQHandler(void)
{
    /* Framing error */
    if (USART1->SR & USART_SR_FE)
    {
        frameFlag = 1;
    }

    /* Noise error */
    if (USART1->SR & USART_SR_NE)
    {
        neFlag = 1;
    }

    /* RX not empty */
    if (USART1->SR & USART_SR_RXNE)
    {
        uint8_t byte = USART1->DR;
        if (!frame_ready)   // ignore bytes while previous frame is pending
        {
            request_frame[rx_index++] = byte;

            if (rx_index >= 8)
            {
                frame_ready = 1;
                rx_index = 0;
            }
        }
    }
}

/**
 * Read sensor value based on input address.
 * input_address: 0x03 → temperature-humidity (dht22)
 *                0x04 → TVOC-eCO2 (grove sgp30)
 * Returns sensor value as int32_t, or -9999 if unknown address
 */
int32_t read_sensor(uint8_t input_address)
{     switch(input_address) {         case 0x03:        		read_dht22_safe();        		update_absolute_humidity();        		print_environment(); // for debugging
        		uint16_t temp16 = (uint16_t)g_temperature_x100;        		uint16_t hum16  = (uint16_t)g_humidity_x100;
        		return ((int32_t)temp16 << 16) | hum16; // temperature-humidity
         case 0x04:    			measure_and_print_iaq();
    			uint16_t eco2  = iaq.eco2_ppm;    			uint16_t tvoc  = iaq.tvoc_ppb;
    			return ((int32_t)eco2 << 16) | tvoc; // eco2-tvoc
         default:             return -9999;                        // invalid address     }
}
