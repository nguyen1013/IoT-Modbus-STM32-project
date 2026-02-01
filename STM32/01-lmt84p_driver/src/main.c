/*LMT84p
*/

/* Includes */
#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>
#include <stdlib.h>

#include "timer.h"
#include "usart2.h"
#include "./sensors/adc_lmt84lp.h"

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
int main(void)
{
	/* Configure the system clock to 32 MHz and update SystemCoreClock */
	SetSysClock();
	SystemCoreClockUpdate();
	USART2_Init();
	ADC_init();

  /* Infinite loop */
	while (1)
	{
	    int32_t temp100 = read_lmt84lp_celsius_x100();

	    char buf[64];

	    int t_int  = temp100 / 100;
	    int t_frac = abs(temp100 % 100);   // two decimal digits

	    if (t_frac < 10)
	        sprintf(buf, "Temp: %d.0%d C\r\n", t_int, t_frac);
	    else
	        sprintf(buf, "Temp: %d.%d C\r\n",  t_int, t_frac);

	    USART2_WriteString(buf);

	    delay_ms(1000);
	}
	return 0;
}
