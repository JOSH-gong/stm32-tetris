#pragma once
#include "stm32f10x.h"

void usart1_init(uint32_t pclk2_MHz, uint32_t baud);
void usart2_init(uint32_t pclk1_MHz, uint32_t baud);

// 发送以 0 结尾的 C 字符串
void usart_print(uint8_t usart_id, const char *s);       // delay 版（2.3前半）
void usart_print_txe(uint8_t usart_id, const char *s);   // TXE 轮询版（2.3核心）
