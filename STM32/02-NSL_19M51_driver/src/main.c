/*This code tests NUCLEO-L152RE board transmitter UART communication by using
 9600 BAUD and float print with sprintf
 */

/* Includes */
#include <stddef.h>
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>

#include "timer.h"
#include "usart.h"
#include "adc_nsl19m51.h"

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
	ADC1_init();
	USART2_write('A');

	/* Infinite loop */
	while (1) {
		uint32_t lux100 = read_NLS19M51_time_100();

		uint32_t lux_int = lux100 / 100;
		uint32_t lux_dec = lux100 % 100;

		char buf[64];

		if (lux_dec < 10)
			sprintf(buf, "Lux=%d.0%d", (int) lux_int, (int) lux_dec);
		else
			sprintf(buf, "Lux=%d.%d", (int) lux_int, (int) lux_dec);

		USART2_WriteString(buf);
		USART2_write('\r');
		USART2_write('\n');

		delay_ms(1000);
	}
	return 0;
}
