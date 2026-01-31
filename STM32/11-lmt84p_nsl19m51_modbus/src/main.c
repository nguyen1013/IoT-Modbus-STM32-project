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
 * Clear buffer. This is used only for receiver buffer, hence fixed size (8 bytes)
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

	USART1_Init(); // ModBus
	USART2_Init(); // Used as debugging terminal
	ADC_init();

	/* TODO - Add your application code here */

	USART1->CR1 |= 0x0020;			//enable RX interrupt
	NVIC_EnableIRQ(USART1_IRQn); 	//enable interrupt in NVIC
	__enable_irq();					//global enable IRQs, M3_Generic_User_Guide p135

	RCC->AHBENR|=1; 				//GPIOA ABH bus clock ON. p154
	GPIOA->MODER&=~0x00000C00;		//clear (input reset state for PA5). p184
	GPIOA->MODER|=0x400; 			//GPIOA pin 5 to output. p184

	char received_frame[8]={0};
	/* Infinite loop */
	unsigned short int crc=0; //16 bitts
	char crc_high_byte=0;
	char crc_low_byte=0;

	char *framingErrorString = "Framing Error Detected";
	char *noiseErrorString = "Noise Error Detected";

	while (1)
	{
		if (frameFlag == 1)
		{
			/* Here we have encountered a framing erorr. It should not occur, but if it does, here we handle it */
			write_debug_msg(framingErrorString, 22);
			frameFlag = 0;
			clear_buffer(received_frame);
		}
		if (neFlag == 1)
		{
			/* If we have noise error in communication, we handle it here */
			write_debug_msg(noiseErrorString, 22);
			neFlag = 0;
			clear_buffer(received_frame);
		}
		if (mFlag == 1 || mFlag == 2) {
		    // Determine slave address
		    uint8_t slave_addr = (mFlag == 1) ? 0x01 : 0x02;

		    // Read 7 bytes from USART1
		    read_7_bytes_from_usartx(&received_frame[1]);
		    received_frame[0] = slave_addr;

		    // Compute CRC
		    crc = CRC16(received_frame, 6);
		    crc_high_byte = crc >> 8;
		    crc_low_byte  = crc & 0xFF;

		    // Verify CRC
		    if (received_frame[7] == crc_high_byte &&
		        received_frame[6] == crc_low_byte)
		    {
		        if (received_frame[3] == 0x01) {
		            // Read sensor value dynamically
		            int32_t sensor_value = read_sensor(slave_addr);
		            if (sensor_value != -9999) {
		                respond_frame(slave_addr, sensor_value);
		            }
		        }
		    }

		    mFlag = 0;
		    USART1->CR1 |= 0x0020;  // Re-enable RX interrupt
		}
		else if(mFlag==3) //wrong slave address
		{
			wrong_slave_address();

			received_frame[0] = 0;
		}
	}

	return 0;
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
		received_slave_address=USART1->DR;
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
            return read_lmt84lp_celsius_x10();  // temperature
        case 0x02:
            return read_NLS19M51_lux();         // luminance
        default:
            return -9999;                        // invalid address
    }
}

