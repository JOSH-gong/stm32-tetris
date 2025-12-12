#pragma once
#include "stm32f10x.h"

// 72MHz SYSCLK, APB2=72MHz, APB1=36MHz
void clocktree_init(void);
