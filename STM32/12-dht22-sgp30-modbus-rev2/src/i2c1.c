#include "stm32l1xx.h"
#include "i2c1.h"

#define I2C_TIMEOUT 10000

/* ================= HELPERS ================= */
static int I2C_WaitFlagSet(volatile uint32_t *reg, uint32_t mask) {
    int timeout = I2C_TIMEOUT;
    while (!(*reg & mask) && timeout--) { __NOP(); }
    return (timeout > 0);
}

static int I2C_WaitFlagClear(volatile uint32_t *reg, uint32_t mask) {
    int timeout = I2C_TIMEOUT;
    while ((*reg & mask) && timeout--) { __NOP(); }
    return (timeout > 0);
}

/* ================= I2C INITIALIZATION ================= */
void I2C1_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->AFR[1] &= ~0x000000FF;
    GPIOB->AFR[1] |= 0x00000044;
    GPIOB->MODER &= ~0x000F0000;
    GPIOB->MODER |= 0x000A0000;
    GPIOB->OTYPER |= 0x00000300;
    GPIOB->PUPDR &= ~0x000F0000;

    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 = 32;      // Peripheral clock 32 MHz
    I2C1->CCR = 160;     // 100 kHz
    I2C1->TRISE = 33;

    I2C1->CR1 |= I2C_CR1_PE;
}

/* ================= I2C WRITE ================= */
void I2C1_Write(uint8_t address, uint8_t command, int n, uint8_t *data) {
    volatile int tmp;
    int i;

    if (!I2C_WaitFlagClear(&I2C1->SR2, I2C_SR2_BUSY)) return;

    I2C1->CR1 |= I2C_CR1_START;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_SB)) return;

    I2C1->DR = address << 1;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_ADDR)) return;
    tmp = I2C1->SR2; // clear ADDR

    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_TXE)) return;
    I2C1->DR = command;

    for (i = 0; i < n; i++) {
        if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_TXE)) return;
        I2C1->DR = data[i];
        if (I2C1->SR1 & I2C_SR1_AF) {
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            return;
        }
    }

    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_BTF)) return;
    I2C1->CR1 |= I2C_CR1_STOP;
}

/* ================= I2C BYTE WRITE ================= */
void I2C1_ByteWrite(uint8_t address, uint8_t command) {
	I2C1_Write(address, command, 0, 0);
}

/* ================= I2C READ ================= */
void I2C1_Read(uint8_t address, uint8_t command, int n, uint8_t *data) {
    volatile int tmp;

    if (!I2C_WaitFlagClear(&I2C1->SR2, I2C_SR2_BUSY)) return;

    // START (write phase)
    I2C1->CR1 |= I2C_CR1_START;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_SB)) return;

    I2C1->DR = address << 1;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_ADDR)) return;
    tmp = I2C1->SR2; // clear ADDR

    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_TXE)) return;
    I2C1->DR = command;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_TXE)) return;

    // Repeated START (read phase)
    I2C1->CR1 |= I2C_CR1_START;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_SB)) return;

    I2C1->DR = (address << 1) | 1;
    if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_ADDR)) return;

    if (n > 1) I2C1->CR1 |= I2C_CR1_ACK;
    else I2C1->CR1 &= ~I2C_CR1_ACK;

    tmp = I2C1->SR2; // clear ADDR

    while (n > 0) {
        if (n == 1) {
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |= I2C_CR1_STOP;
        }

        if (!I2C_WaitFlagSet(&I2C1->SR1, I2C_SR1_RXNE)) return;

        *data++ = I2C1->DR;
        n--;
    }
}

void I2C1_BusRecover(void)
{
    int i;

    /* 1. Disable I2C1 */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /* 2. Configure PB8, PB9 as GPIO open-drain outputs */
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    /* MODER: output mode (01) */
    GPIOB->MODER &= ~(0xF << (8 * 2));
    GPIOB->MODER |=  (0x5 << (8 * 2));   // PB8, PB9 = output

    /* OTYPER: open-drain */
    GPIOB->OTYPER |= (1 << 8) | (1 << 9);

    /* PUPDR: pull-up */
    GPIOB->PUPDR &= ~(0xF << (8 * 2));
    GPIOB->PUPDR |=  (0x5 << (8 * 2));

    /* Set both lines high */
    GPIOB->BSRR = (1 << 8) | (1 << 9);

    /* 3. Clock out 9 pulses on SCL */
    for (i = 0; i < 9; i++) {
        GPIOB->BSRR = (1 << (8 + 16));   // SCL low
        delay_ms(1);
        GPIOB->BSRR = (1 << 8);          // SCL high
        delay_ms(1);
    }

    /* 4. Generate STOP: SDA low → SCL high → SDA high */
    GPIOB->BSRR = (1 << (9 + 16));   // SDA low
    delay_ms(1);
    GPIOB->BSRR = (1 << 8);          // SCL high
    delay_ms(1);
    GPIOB->BSRR = (1 << 9);          // SDA high
    delay_ms(1);

    /* 5. Restore PB8/PB9 to AF4 (I2C1) */
    GPIOB->MODER &= ~(0xF << (8 * 2));
    GPIOB->MODER |=  (0xA << (8 * 2));   // AF mode

    GPIOB->AFR[1] &= ~0x000000FF;
    GPIOB->AFR[1] |=  0x00000044;        // AF4 for PB8, PB9

    /* 6. Software reset I2C1 */
    I2C1->CR1 |=  I2C_CR1_SWRST;
    delay_ms(1);
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    /* 7. Re-enable I2C1 */
    I2C1->CR1 |= I2C_CR1_PE;
}

