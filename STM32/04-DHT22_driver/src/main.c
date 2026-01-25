#include "stm32l1xx.h"
#include "nucleo152start.h"
#include <stdio.h>

#include "usart.h"
#include "one-wire-dht22.h"

/* === Constants === */
#define PA5_PIN   (1 << 5)   // Onboard LED

/* === Function Prototypes === */
void SetSysClock(void);

/* === MAIN === */
int main(void) {
	__disable_irq();

	SetSysClock();
	SystemCoreClockUpdate();
	DHT22_init();
	USART2_Init();

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	// PA5 = LED output
	GPIOA->MODER &= ~(3 << (5 * 2));
	GPIOA->MODER |= (1 << (5 * 2));

	__enable_irq();

	USART2_WriteString("STM32L152RE + DHT22 on PB3\r\n");

	while (1) {
	    int32_t* values = read_DHT22_value_time_100();

	    int32_t hum100  = values[0];
	    int32_t temp100 = values[1];

	    if (hum100 == 0 && temp100 == 0) {
	        USART2_WriteString("Sensor error\r\n");
	    } else {
	        char buffer[64];

	        int rh_int   = hum100 / 100;
	        int rh_dec_1 = (hum100 % 100) / 10;

	        int t_int    = temp100 / 100;
	        int t_dec_1  = abs(temp100 % 100) / 10;

	        sprintf(buffer,
	                "RH: %d.%d %%  Temp: %d.%d C\r\n",
	                rh_int, rh_dec_1,
	                t_int, t_dec_1);

	        USART2_WriteString(buffer);
	    }

	    delay_ms(1000);
	}
}
