/* ps2.h - PS/2 keyboard receiver (uses PC11 CLK, PC10 DATA)
 * Provides a tiny API to get the last "make" scancode (non-zero).
 */
#ifndef __PS2_H
#define __PS2_H

#include "stm32f10x.h"

/* Initialize PS/2 GPIO and EXTI to receive keyboard scancodes */
void PS2_Init(void);

/* Return last make scancode (non-zero) and clear it. Non-blocking.
 * bit8=1 means the code came after an 0xE0 prefix (extended key).
 */
uint16_t PS2_GetLastMakeCode(void);

/* Polling step: call frequently from main loop to sample CLK falling edges */
void PS2_Poll(void);

#endif /* __PS2_H */
