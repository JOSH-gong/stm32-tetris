// ===== 1) 头文件（提供寄存器/位定义与 __NOP）=====
#include "stm32f10x.h"
#include <stdint.h>   /* 提供 uint32_t 的定义（C90 环境也可用） */

#include <stddef.h>
/* FONT.H contains the full font array definition which must not be
  re-defined in multiple compilation units. Instead of including
  FONT.H here (which the project owner reverted), reference the
  font array as an extern symbol so we don't produce a duplicate
  definition when other modules include FONT.H. */
extern const unsigned char asc2_1608[95][16];


// ===== 2) 基本类型别名（你在代码里用到了 u32/u16/vu16）=====
typedef uint32_t u32;
typedef uint16_t u16;
typedef volatile uint16_t vu16;

// ===== 3) 先给将要用到的函数“提前声明”=====
// 你在 Init() 里先用到了它们，C 里需要事先有原型
void IERG3810_TFTLCD_WrReg(u16 regval);
void IERG3810_TFTLCD_WrData(u16 data);
/* forward declare parameter/setup function to avoid implicit declaration */
void LCD_Set9341_Parameter(void);
// ===== 4) delay 的实现（供 lcd_init 等内部使用）=====
void Delay(volatile uint32_t t)
{
  while (t--) __NOP();
}




#define LCD_BASE  ((u32)0x6C000000)
#define LCD_REG   (*((vu16 *)(LCD_BASE)))           // A10 = 0  -> 命令
#define LCD_RAM   (*((vu16 *)(LCD_BASE | 0x0800)))  // A10 = 1  -> 数据

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0

void IERG3810_TFTLCD_WrReg(u16 reg){  LCD_REG = reg; }
void IERG3810_TFTLCD_WrData(u16 dat){ LCD_RAM = dat; }

void IERG3810_TFTLCD_Init(void);
void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color);
void IERG3810_TFTLCD_SetWindow(u16 x0, u16 x1, u16 y0, u16 y1);
void IERG3810_TFTLCD_FillRectangle(u16 color, u16 x, u16 w, u16 y, u16 h);

extern void IERG3810_TFTLCD_ShowString(u16 x, u16 y, const char* s, u16 color);


void IERG3810_TFTLCD_Init(void)
{
  RCC->AHBENR  |= 1<<8;
  RCC->APB2ENR |= 1<<3 | 1<<5 | 1<<6 | 1<<8;

  GPIOB->CRL &= 0xFFFFFFF0; GPIOB->CRL |= 0x3; GPIOB->BSRR = 1<<0;

  GPIOD->CRH &= 0x00FFF000; GPIOD->CRH |= 0xBB000BBB;
  GPIOD->CRL &= 0xFF00FF00; GPIOD->CRL |= 0x00BB00BB;

  GPIOE->CRH &= 0x00000000; GPIOE->CRH |= 0xBBBBBBBB;
  GPIOE->CRL &= 0x0FFFFFFF; GPIOE->CRL |= 0xB0000000;

  GPIOG->CRH &= 0xFFF0FFFF; GPIOG->CRH |= 0x000B0000;
  GPIOG->CRL &= 0xFFFFFFF0; GPIOG->CRL |= 0x0000000B;

  FSMC_Bank1->BTCR[6]  = 0x00000000;
  FSMC_Bank1->BTCR[7]  = 0x00000000;
  FSMC_Bank1E->BWTR[6] = 0x00000000;

  FSMC_Bank1->BTCR[6] |= 1<<12 | 1<<14 | 1<<4;
  FSMC_Bank1->BTCR[7] |= 1<<0 | (0xF<<8);
  FSMC_Bank1E->BWTR[6]|= (0<<0) | (3<<8);
  FSMC_Bank1->BTCR[6] |= 1<<0;

  Delay(1000000);
  LCD_Set9341_Parameter();
}

void LCD_Set9341_Parameter(void)
{
  IERG3810_TFTLCD_WrReg(0x01); Delay(10000);
  IERG3810_TFTLCD_WrReg(0x11); Delay(120000);
  IERG3810_TFTLCD_WrReg(0x3A); IERG3810_TFTLCD_WrData(0x55);
  IERG3810_TFTLCD_WrReg(0x29);
  IERG3810_TFTLCD_WrReg(0x36); IERG3810_TFTLCD_WrData(0xC8);
}



void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color)
{
  if (x>=240 || y>=320) return;
  IERG3810_TFTLCD_WrReg(0x2A);
  IERG3810_TFTLCD_WrData(x>>8); IERG3810_TFTLCD_WrData(x&0xFF);
  IERG3810_TFTLCD_WrData(x>>8); IERG3810_TFTLCD_WrData(x&0xFF);
  IERG3810_TFTLCD_WrReg(0x2B);
  IERG3810_TFTLCD_WrData(y>>8); IERG3810_TFTLCD_WrData(y&0xFF);
  IERG3810_TFTLCD_WrData(y>>8); IERG3810_TFTLCD_WrData(y&0xFF);
  IERG3810_TFTLCD_WrReg(0x2C);
  IERG3810_TFTLCD_WrData(color);
}

void IERG3810_TFTLCD_SetWindow(u16 x0, u16 x1, u16 y0, u16 y1)
{
  IERG3810_TFTLCD_WrReg(0x2A);
  IERG3810_TFTLCD_WrData(x0>>8); IERG3810_TFTLCD_WrData(x0&0xFF);
  IERG3810_TFTLCD_WrData(x1>>8); IERG3810_TFTLCD_WrData(x1&0xFF);
  IERG3810_TFTLCD_WrReg(0x2B);
  IERG3810_TFTLCD_WrData(y0>>8); IERG3810_TFTLCD_WrData(y0&0xFF);
  IERG3810_TFTLCD_WrData(y1>>8); IERG3810_TFTLCD_WrData(y1&0xFF);
  IERG3810_TFTLCD_WrReg(0x2C);
}

void IERG3810_TFTLCD_FillRectangle(u16 color, u16 x, u16 w, u16 y, u16 h)
{
  u16 x1, y1; u32 pixels, i;
  if (x>=240 || y>=320) return;
  x1 = (x+w-1 >= 240)? 239 : (u16)(x+w-1);
  y1 = (y+h-1 >= 320)? 319 : (u16)(y+h-1);
  IERG3810_TFTLCD_SetWindow(x, x1, y, y1);
  pixels = (u32)(x1-x+1)*(u32)(y1-y+1);
  for (i=0;i<pixels;i++) IERG3810_TFTLCD_WrData(color);
}


/* ===== Text rendering (8x16 font from FONT.H) ===== */
void IERG3810_TFTLCD_ShowChar(u16 x, u16 y, unsigned char ascii, u16 color)
{
  const unsigned char *glyph;
  int col, row;
  u16 mask;

  if (ascii < 32 || ascii > 126) return;
  glyph = asc2_1608[ascii - 32];                 /* 8 列 × 每列2字节 = 16字节 */

  for (col = 0; col < 8; col++) {
    mask = ((u16)glyph[col*2] << 8) | (u16)glyph[col*2 + 1];  /* 高字节在前 */
    for (row = 0; row < 16; row++) {
      if (mask & (1u << (15 - row))) {           /* bit15=最上 → 翻到 y+(15-row) */
        IERG3810_TFTLCD_DrawDot((u16)(x + col), (u16)(y + (15 - row)), color);
      }
    }
  }
}

/* 显示 ASCII 字符串（等宽 8×16；自动换行） */
void IERG3810_TFTLCD_ShowString(u16 x, u16 y, const char* s, u16 color)
{
  u16 cx = x, cy = y;
  char c;
  while ((c = *s++) != '\0') {
    if (c == '\n') { cy = (u16)(cy + 16); cx = x; continue; }
    IERG3810_TFTLCD_ShowChar(cx, cy, (unsigned char)c, color);
    cx = (u16)(cx + 8);
    if (cx + 8 > 320) { cx = x; cy = (u16)(cy + 16); }
  }
}
