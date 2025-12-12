#ifndef __IERG3810_LED_H
#define __IERG3810_LED_H
#include "stm32f10x.h"

void IERG3810_LED_Init(void);
void IERG3810_LED_DS0_On(void);
void IERG3810_LED_DS0_Off(void);
void IERG3810_LED_DS1_On(void);
void IERG3810_LED_DS1_Off(void);
void IERG3810_LED_Flash_DS0(uint32_t delay_ticks, uint32_t cycles);
void IERG3810_LED_Flash_DS1(uint32_t delay_ticks, uint32_t cycles);

#endif
