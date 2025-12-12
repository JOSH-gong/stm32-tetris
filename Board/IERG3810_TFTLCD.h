#ifndef __IERG3810_TFTLCD_H
#define __IERG3810_TFTLCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include <stdint.h>

/* basic type aliases used in the C file */
typedef uint32_t u32;
typedef uint16_t u16;
typedef volatile uint16_t vu16;

/* LCD register/data address (as used in the C file) */
#define LCD_BASE  ((u32)0x6C000000)
#define LCD_REG   (*((vu16 *)(LCD_BASE)))           /* A10 = 0  -> command */
#define LCD_RAM   (*((vu16 *)(LCD_BASE | 0x0800)))  /* A10 = 1  -> data */

/* common color definitions */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0

/* Delay used by the implementation */
void Delay(volatile uint32_t t);

/* Low-level write helpers (implemented in .c) */
void IERG3810_TFTLCD_WrReg(u16 reg);
void IERG3810_TFTLCD_WrData(u16 dat);

/* Initialization and control */
void IERG3810_TFTLCD_Init(void);
/* LCD parameter helper (defined in .c) */
void LCD_Set9341_Parameter(void);

/* Drawing primitives */
void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color);
void IERG3810_TFTLCD_SetWindow(u16 x0, u16 x1, u16 y0, u16 y1);
void IERG3810_TFTLCD_FillRectangle(u16 color, u16 x, u16 w, u16 y, u16 h);

/* Higher-level text/strings */
void IERG3810_TFTLCD_ShowString(u16 x, u16 y, const char* s, u16 color);

#ifdef __cplusplus
}
#endif

#endif /* __IERG3810_TFTLCD_H */
