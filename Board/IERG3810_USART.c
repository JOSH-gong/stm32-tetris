#include "IERG3810_USART.h"

static void _set_brr(volatile USART_TypeDef *U, uint32_t pclk_MHz, uint32_t baud)
{
    float t = (float)(pclk_MHz*1000000) / (baud*16);
    uint16_t man = (uint16_t)t;
    uint16_t fra = (uint16_t)((t - man)*16);
    U->BRR = (man<<4) + fra;     // 8N1 缺省
    U->CR1 = 0x2008;             // UE=1, TE=1
}

void usart1_init(uint32_t pclk2, uint32_t baud)
{
    RCC->APB2ENR |= (1<<2) | (1<<14); // IOPAEN | USART1EN
    GPIOA->CRH &= ~(0xFF<<4);
    GPIOA->CRH |=  (0x0B<<4);   // PA9  AF_PP 50MHz
    GPIOA->CRH |=  (0x04<<8);   // PA10 IN_FLOATING
    RCC->APB2RSTR |=  (1<<14); RCC->APB2RSTR &= ~(1<<14);
    _set_brr(USART1, pclk2, baud);
}

void usart2_init(uint32_t pclk1, uint32_t baud)
{
    RCC->APB2ENR |= (1<<2);     // IOPAEN
    RCC->APB1ENR |= (1<<17);    // USART2EN
    GPIOA->CRL &= ~(0xFF<<8);
    GPIOA->CRL |=  (0x0B<<8);   // PA2  AF_PP 50MHz
    GPIOA->CRL |=  (0x04<<12);  // PA3  IN_FLOATING
    RCC->APB1RSTR |=  (1<<17); RCC->APB1RSTR &= ~(1<<17);
    _set_brr(USART2, pclk1, baud);
}

void usart_print(uint8_t usart_id, const char *s)
{
    volatile USART_TypeDef *U = (usart_id == 1) ? USART1 : USART2;
    while (*s) {
        U->DR = (uint8_t)(*s++);

        // —— C90 兼容的延时写法 —— 
        {
            volatile uint32_t i;         // 先在块开头声明
            for (i = 0; i < 80000; ++i) {
                __asm("nop");            // 防优化的空操作（Keil 支持）
            }
        }
    }
}


void usart_print_txe(uint8_t usart_id, const char *s)
{
    volatile USART_TypeDef *U = (usart_id==1)?USART1:USART2;
    while (*s) {
        while (!(U->SR & (1<<7)));   // TXE
        U->DR = (uint8_t)(*s++);
    }
    while (!(U->SR & (1<<6)));       // TC
}
