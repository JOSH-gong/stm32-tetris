#include "IERG3810_Buzzer.h"
#include "stm32f10x.h"

void IERG3810_Buzzer_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* PB8 -> push-pull output, 50MHz */
    GPIOB->CRH &= ~(0xF << 0);
    GPIOB->CRH |=  (0x3 << 0);

    GPIOB->BRR = (1 << 8); /* default low */
}

static void IERG3810_Buzzer_Toggle(uint32_t delay_ticks, uint32_t cycles)
{
    uint32_t i;
    for (i = 0; i < cycles; ++i) {
        GPIOB->BSRR = (1 << 8); /* high -> sound on */
        Delay(delay_ticks);
        GPIOB->BRR = (1 << 8);  /* low -> sound off */
        Delay(delay_ticks);
    }
}

void IERG3810_Buzzer_BeepShort(void)
{
    IERG3810_Buzzer_Toggle(1000, 160); /*明显一点的提示音*/
}

void IERG3810_Buzzer_BeepGameOver(void)
{
    /* two descending tones */
    IERG3810_Buzzer_Toggle(800, 150);
    Delay(30000);
    IERG3810_Buzzer_Toggle(1500, 120);
}

void IERG3810_Buzzer_BeepStart(void)
{
    /* 一个开场提示：两声短促 beep */
    IERG3810_Buzzer_Toggle(700, 80);
    Delay(15000);
    IERG3810_Buzzer_Toggle(700, 80);
}
