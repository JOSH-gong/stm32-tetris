#ifndef __IERG3810_BUZZER_H
#define __IERG3810_BUZZER_H
#include "stm32f10x.h"

void IERG3810_Buzzer_Init(void);
void IERG3810_Buzzer_BeepShort(void);
void IERG3810_Buzzer_BeepGameOver(void);
void IERG3810_Buzzer_BeepStart(void);

#endif
