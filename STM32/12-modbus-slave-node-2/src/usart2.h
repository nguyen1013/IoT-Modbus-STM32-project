#ifndef USART_H
#define USART_H

#include <stdint.h>
#include "stm32l1xx.h"

void USART2_Init(void);

/* Write a null-terminated string over USART2 */
void USART2_WriteString(const char *s);

/* Write a array of chars (no null-terminated) over USART2 */
void USART2_WriteCharArray(const char *str, int32_t maxchars);

/* Write a single character */
void USART2_write(char c);

char USART2_read(void);

#endif /* USART_H */
