#include "IERG3810_Clock.h"

void clocktree_init(void)
{
    uint8_t temp = 0;

    // 8MHz HSE on
    RCC->CR   |=  (1<<16);
    while (!(RCC->CR & (1<<17)));

    // AHB=/1, APB2=/1, APB1=/2; PLLSRC=HSE; PLLMUL=x9
    RCC->CFGR &= 0xF8FF0000;
    RCC->CFGR |= (0x4<<8);   // PPRE1=/2
    RCC->CFGR |= (1<<16);    // PLLSRC=HSE
    RCC->CFGR |= (7<<18);    // PLL x9 -> 72MHz

    // Flash latency & prefetch
    FLASH->ACR |= 0x32;

    // Enable PLL and wait
    RCC->CR   |= (1<<24);
    while (!(RCC->CR & (1<<25)));

    // Switch SYSCLK to PLL
    RCC->CFGR |= 0x00000002;
    do { temp = (RCC->CFGR >> 2) & 0x03; } while (temp != 0x02);
}
