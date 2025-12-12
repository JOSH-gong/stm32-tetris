#include "IERG3810_LED.h"
#include "stm32f10x.h"

void Delay(volatile uint32_t t); /* extern from TFT driver */

void IERG3810_LED_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;

    GPIOB->CRL &= ~(0xF << 20);
    GPIOB->CRL |=  (0x3 << 20);

    GPIOE->CRL &= ~(0xF << 20);
    GPIOE->CRL |=  (0x3 << 20);

    GPIOB->BSRR = (1 << 5);
    GPIOE->BSRR = (1 << 5);
}

void IERG3810_LED_DS0_On(void)  { GPIOB->BRR  = (1 << 5); }
void IERG3810_LED_DS0_Off(void) { GPIOB->BSRR = (1 << 5); }
void IERG3810_LED_DS1_On(void)  { GPIOE->BRR  = (1 << 5); }
void IERG3810_LED_DS1_Off(void) { GPIOE->BSRR = (1 << 5); }

void IERG3810_LED_Flash_DS0(uint32_t delay_ticks, uint32_t cycles)
{
    uint32_t i;
    for (i = 0; i < cycles; ++i) {
        IERG3810_LED_DS0_On();
        Delay(delay_ticks);
        IERG3810_LED_DS0_Off();
        Delay(delay_ticks);
    }
}

void IERG3810_LED_Flash_DS1(uint32_t delay_ticks, uint32_t cycles)
{
    uint32_t i;
    for (i = 0; i < cycles; ++i) {
        IERG3810_LED_DS1_On();
        Delay(delay_ticks);
        IERG3810_LED_DS1_Off();
        Delay(delay_ticks);
    }
}
