/* ps2.c - simple PS/2 receiver implementation
 * Moves EXTI15_10 IRQ handler here and provides PS2_Init and
 * PS2_GetLastMakeCode() for main program to consume.
 */

#include "ps2.h"

/* local state */
static volatile uint8_t  ps2_bitcnt = 0;
static volatile uint16_t ps2_shift = 0;
static volatile uint8_t  ps2_break = 0;
static volatile uint8_t  ps2_ext = 0;        /* saw 0xE0 prefix */
static volatile uint8_t  ps2_last_make = 0;  /* last make code, 0 if none */
static volatile uint8_t  ps2_last_is_ext = 0;
static volatile uint16_t ps2_idle_ticks = 0; /* polls without CLK edge */

void PS2_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

    /* PC11: CLK 输入上拉 (CRH[15:12]) */
    GPIOC->CRH &= ~(0xF << 12);
    GPIOC->CRH |=  (0x2 << 14);
    GPIOC->ODR |=  (1U << 11);

    /* PC10: DATA 输入上拉 (CRH[11:8]) */
    GPIOC->CRH &= ~(0xF << 8);
    GPIOC->CRH |=  (0x2 << 10);
    GPIOC->ODR |=  (1U << 10);

    /* Do NOT enable EXTI/NVIC here: using polling mode to avoid ISR conflicts */
    /* Map EXTI11->PortC kept for reference but not used when polling */
    AFIO->EXTICR[2] &= ~(0xF << 12);
    AFIO->EXTICR[2] |=  (0x2 << 12);
}

/* Polling-based step: call frequently from main loop to detect CLK falling edge */
void PS2_Poll(void)
{
    static uint32_t prev_clk = 1;
    uint32_t clk = (GPIOC->IDR >> 11) & 0x1U;
    uint32_t data_bit;

    /* detect falling edge */
    if (prev_clk == 1 && clk == 0) {
        ps2_idle_ticks = 0; /* saw activity */
        /* sample data */
        data_bit = (GPIOC->IDR >> 10) & 0x1U;
        ps2_shift |= (uint16_t)(data_bit & 0x1U) << ps2_bitcnt;
        ps2_bitcnt++;

        if (ps2_bitcnt >= 11) {
            uint16_t frame = ps2_shift;
            uint8_t sc = (uint8_t)((frame >> 1) & 0xFF);

            ps2_bitcnt = 0;
            ps2_shift  = 0;

            /* basic validity: start=0, stop=1 */
            if (((frame & 0x001) == 0) && ((frame & 0x400) != 0)) {
                if (sc == 0xE0) {
                    ps2_ext = 1;
                } else if (sc == 0xF0) {
                    ps2_break = 1;
                } else {
                    if (!ps2_break) {
                        ps2_last_make   = sc;
                        ps2_last_is_ext = ps2_ext;
                    }
                    /* reset prefix/break after a full code */
                    ps2_break = 0;
                    ps2_ext   = 0;
                }
            }
        }
    } else {
        /* no edge: count idle polls; if frame stalled, reset state to avoid desync */
        if (ps2_idle_ticks < 1000) ps2_idle_ticks++;
        if (ps2_idle_ticks > 80 && ps2_bitcnt > 0) {
            ps2_bitcnt = 0;
            ps2_shift  = 0;
            ps2_break  = 0;
            ps2_ext    = 0;
        }
    }

    prev_clk = clk;
}

/* Return and clear the last make code (0 if none) */
uint16_t PS2_GetLastMakeCode(void) {
    uint16_t v = (uint16_t)ps2_last_make | (ps2_last_is_ext ? 0x0100 : 0);
    ps2_last_make = 0;
    ps2_last_is_ext = 0;
    return v;
}
