#include "usart1.h"
#include "stm32l1xx.h"


/**
 * Initialize Modbus pins for UART1
 */
void USART1_Init(void)
{
	RCC->APB2ENR|=(1<<14);	 	//set bit 14 (USART1 EN) p.156
	RCC->AHBENR|=0x00000001; 	//enable GPIOA port clock bit 0 (GPIOA EN)
	GPIOA->AFR[1]=0x00000700;	//GPIOx_AFRL p.189,AF7 p.177 (AFRH10[3:0])
	GPIOA->AFR[1]|=0x00000070;	//GPIOx_AFRL p.189,AF7 p.177 (AFRH9[3:0])
	GPIOA->MODER|=0x00080000; 	//MODER2=PA9(TX)D8 to mode 10=alternate function mode. p184
	GPIOA->MODER|=0x00200000; 	//MODER2=PA10(RX)D2 to mode 10=alternate function mode. p184

	USART1->BRR = 0x00000D05;	//9600 BAUD and crystal 32MHz. p710, D05
	USART1->CR1 = 0x00000008;	//TE bit. p739-740. Enable transmit
	USART1->CR1 |= 0x00000004;	//RE bit. p739-740. Enable receiver
	USART1->CR1 |= 0x00002000;	//UE bit. p739-740. Uart enable

	USART1->CR1 |= USART_CR1_SBK; // SBK bit. Send break enabled

	USART1->CR2 = 0x00; // reset

	USART1->CR3 = 0;   // Set to default state
	USART1->CR3 |= 1;  // Enable error interrupt,  p744
	/* Error Interrupt Enable Bit is required to enable interrupt generation in case of a framing
	error, overrun error or noise flag (FE=1 or ORE=1 or NF=1 in the USART_SR register) in
	case of Multi Buffer Communication (DMAR=1 in the USART_CR3 register).
	 */
}

void USART1_write(char data)
{
	//wait while TX buffer is empty
	while(!(USART1->SR&0x0080)){} 	//TXE: Transmit data register empty. p736-737
	USART1->DR=(data);			//p739
}

char USART1_read()
{
	char data=0;
	//wait while RX buffer is data is ready to be read
	while(!(USART1->SR&0x0020)){} 	//Bit 5 RXNE: Read data register not empty
	data=USART1->DR;			//p739
	return data;
}
