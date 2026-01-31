#include "stm32l1xx.h"
#define HSI_VALUE    ((uint32_t)16000000)
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>

#include "usart2.h"
#include "usart1.h"
#include "timer.h"
#include "modbus_slave.h"
#include "./sensors/adc_lmt84lp.h"
#include "./sensors/adc_nsl19m51.h"

/* Private typedef */
/* Private define  */
/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

int32_t read_sensor(uint8_t input_address);

/************************************************************************/
/* Flags are volatile, since the flag value can change during interrupt */
/* handler. Therefore compiler can handle it correctly.                 */
/************************************************************************/
volatile char mFlag=0;
volatile uint8_t neFlag = 0;
volatile uint8_t frameFlag = 0;

/*
 * Clear buffer
 */
void clear_buffer(char *b)
{
	for (int32_t i=0;i<8;i++)
		b[i] = 0;
}

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void)
{
	__disable_irq();			//global disable IRQs, M3_Generic_User_Guide p135.
	/* Configure the system clock to 32 MHz and update SystemCoreClock */
    SetSysClock();
    SystemCoreClockUpdate();

    USART1_Init();   // Modbus RTU
    USART2_Init();   // Debug
    ADC_init();
    LED_Init();

    USART1->CR1 |= USART_CR1_RXNEIE;   // RX interrupt enable
    NVIC_EnableIRQ(USART1_IRQn);

    __enable_irq();

    /* Modbus request buffer:
       slave + func + addrH + addrL + qtyH + qtyL + crcL + crcH = 8 bytes */
    uint8_t request_frame[8] = {0};

    uint16_t crc;
    const char *framingErrorString = "Framing Error Detected";
    const char *noiseErrorString   = "Noise Error Detected";

    while (1)
    {
        if (frameFlag)
        {
            write_debug_msg(framingErrorString, 22);
            frameFlag = 0;
        }

        if (neFlag)
        {
            write_debug_msg(noiseErrorString, 22);
            neFlag = 0;
        }

        /* -------- Modbus request received -------- */
        if (mFlag == 1 || mFlag == 2)
        {
            uint8_t slave_addr = (mFlag == 1) ? 0x01 : 0x02;

            /* First byte (slave addr) already received in ISR */
            request_frame[0] = slave_addr;

            /* Read remaining 7 bytes */
            read_modbus_frame(&request_frame[1], 7);

            /* CRC check (first 6 bytes) */
            crc = CRC16((char *)request_frame, 6);

            if (request_frame[6] == (crc & 0xFF) &&
                request_frame[7] == (crc >> 8))
            {
                /* Function 0x04, start addr LSB == 0x01 */
                if (request_frame[1] == 0x04 &&
                    request_frame[3] == 0x01)
                {
                    int32_t sensor_value = read_sensor(slave_addr);

                    if (sensor_value != -9999)
                    {
                        respond_frame(slave_addr, sensor_value);
                    }
                }
            }

            mFlag = 0;
            USART1->CR1 |= USART_CR1_RXNEIE;   // re-enable RX interrupt
        }
        else if (mFlag == 3)
        {
            mFlag = 0;
            USART1->CR1 |= USART_CR1_RXNEIE;
        }
    }
}

void USART1_IRQHandler(void)
{
	char received_slave_address=0;

	/**
	 * If there is framing error (physical) in the modbus, we raise a flag here. Since inside
	 * IRQ handler actions should be simple and fast.  We will then investigate the flag in main()
	 */
	if (USART1->SR & USART_SR_FE)
	{
		frameFlag = 1;
	}

	/**
	 * If there is noise error (physical) in the modbus, we raise a flag here. Since inside
	 * IRQ handler actions should be simple and fast. We will then investigate the flag in main()
	 */
	if (USART1->SR & USART_SR_NE)
	{
		neFlag = 1;
	}

	if(USART1->SR & 0x0020) 		//if data available in DR register. p737
	{
		received_slave_address = USART1->DR;
	}

	if (received_slave_address == 0x01) {
	    mFlag = 1;   // adc_lmt84lp
	}
	else if (received_slave_address == 0x02) {
	    mFlag = 2;   // adc_nsl19m51
	}
	else {
	    mFlag = 3;   // ignore
	}

	USART1->CR1 &= ~0x0020;			//disable RX interrupt
}

/**
 * Read sensor value based on input address.
 * input_address: 0x01 → temperature (LMT84LP)
 *                0x02 → luminance (NSL19M51)
 * Returns sensor value as int32_t, or -9999 if unknown address
 */
int32_t read_sensor(uint8_t input_address)
{
    switch(input_address) {
        case 0x01:
            return read_lmt84lp_celsius_x100();  // temperature
        case 0x02:
            return read_NLS19M51_lux();         // luminance
        default:
            return -9999;                        // invalid address
    }
}

