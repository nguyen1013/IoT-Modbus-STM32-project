/*This code tests NUCLEO-L152RE board transmitter UART communication by using
 9600 BAUD and float print with sprintf
 */

/* Includes */
#include <stddef.h>
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>

#include "timer.h"
#include "usart2.h"
#include "./sensors/adc_nsl19m51.h"

/* Private typedef */
/* Private define  */
/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void) {
	/* Configure the system clock to 32 MHz and update SystemCoreClock */
	SetSysClock();
	SystemCoreClockUpdate();
	USART2_Init();
	ADC_init();

	/* Infinite loop */
	while (1) {
		uint16_t lux = read_NLS19M51_lux();

		char buf[64];

		sprintf(buf, "Lux=%d", (int) lux);

		USART2_WriteString(buf);
		USART2_write('\r');
		USART2_write('\n');

		delay_ms(1000);
	}
	return 0;
}
