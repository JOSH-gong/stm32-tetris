#include "tetris_title.h"
#include "stm32f10x.h"
#include "CFONT.H"   /* 中文点阵：HZ16_GONG, HZ16_HOU ... */
#include "FONT.H"    /* ASCII 8x16: asc2_1608 */
#include <stdio.h>

typedef unsigned short u16;
typedef unsigned int   u32;

/* === 声明你在 TFTLCD.c 里已有的函数（使用 TFTLCD.c 中的命名） === */
extern void IERG3810_TFTLCD_Init(void);
extern void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color);

/* === 颜色 & 尺寸（竖屏 240x320） === */
#define COL_BLACK  0x0000
#define COL_WHITE  0xFFFF
#define COL_RED    0xF800
#define COL_GREEN  0x07E0
#define COL_BLUE   0x001F
#define COL_YELLOW 0xFFE0

#define LCD_W 240
#define LCD_H 320

/* 简单延时（只在需要时用，用不到可以不调） */
static void ttl_Delay(volatile u32 t){ while(t--) __NOP(); }

/* ----------------- 基础图形封装：填充矩形 ----------------- */
/* 用很多 drawDot 堆出一块矩形，效率一般，但标题界面够用了 */
static void fill_rect(u16 color, u16 x, u16 w, u16 y, u16 h)
{
    u16 xx, yy;
    for (yy = 0; yy < h; yy++)
    {
        for (xx = 0; xx < w; xx++)
        {
            IERG3810_TFTLCD_DrawDot((u16)(x + xx), (u16)(y + yy), color);
        }
    }
}

/* ----------------- ASCII 8×16 字符/字符串 ----------------- */
static void draw_char8x16(u16 x, u16 y, unsigned char ch, u16 color)
{
    const unsigned char *glyph;
    u16 mask;
    int col, row;

    if (ch < 32 || ch > 126) return;
    glyph = asc2_1608[ch - 32];  /* 8列，每列2字节 */

    for (col = 0; col < 8; col++)
    {
        mask = ((u16)glyph[col*2] << 8) | (u16)glyph[col*2 + 1];
        for (row = 0; row < 16; row++)
        {
            if (mask & (1u << (15 - row)))
            {
                /* 你的坐标系：原点在左下，所以 y+(15-row) */
                IERG3810_TFTLCD_DrawDot((u16)(x + col), (u16)(y + (15 - row)), color);
            }
        }
    }
}

static void draw_string(u16 x, u16 y, const char* s, u16 color)
{
    u16 cx = x;
    char c;
    while ((c = *s++) != '\0')
    {
        if (c == '\n')
        {
            y  = (u16)(y + 16);
            cx = x;
            continue;
        }
        draw_char8x16(cx, y, (unsigned char)c, color);
        cx = (u16)(cx + 8);
        if (cx + 8 > LCD_W)
        {
            cx = x;
            y  = (u16)(y + 16);
        }
    }
}

/* ----------------- 中文 16×16 显示（CFONT.H） ----------------- */
static void showChinChar(const unsigned char *glyph32, u16 x, u16 y, u16 color)
{
    int row, col;
    u16 mask;
    if (!glyph32) return;

    for (row = 0; row < 16; row++)
    {
        mask = ((u16)glyph32[row*2] << 8) | (u16)glyph32[row*2 + 1];
        for (col = 0; col < 16; col++)
        {
            if (mask & (1u << (15 - col)))
            {
                IERG3810_TFTLCD_DrawDot((u16)(x + col), (u16)(y + (15 - row)), color);
            }
        }
    }
}

static void showChinString(u16 x, u16 y, const unsigned char* glyphs[], int count, u16 color)
{
    int i;
    u16 cx = x;

    for (i = 0; i < count; i++)
    {
        showChinChar(glyphs[i], cx, y, color);
        cx = (u16)(cx + 16);     /* 每个汉字 16 像素宽 */
        if (cx + 16 > LCD_W)
        {
            cx = x;
            y  = (u16)(y + 16);
        }
    }
}

/* ============================================================
 *            标题界面：TETRIS + 中文名字 + CUID
 * ============================================================ */
void TETRIS_ShowTitlePage(int high_score)
{
    u16 x, y;

    /* 1. 初始化 LCD，清屏为黑 */
    IERG3810_TFTLCD_Init();
    fill_rect(COL_BLACK, 0, LCD_W, 0, LCD_H);

    /* 2. 顶部英文标题：TETRIS */
    x = (u16)((LCD_W - 6*8) / 2);   /* 6 个字母，每个 8 像素宽 */
    y = 260;
    draw_string(x, y, "TETRIS", COL_YELLOW);

    /* 3. 中间两行：中文名字 + CUID */

    /* --------- 第1行：龚子恒 1155211183 --------- */
    {
        const unsigned char* me[3];
        me[0] = HZ16_GONG;
        me[1] = HZ16_ZI;
        me[2] = HZ16_HENG;

        x = 20;
        y = 200;
        showChinString(x, y, me, 3, COL_YELLOW);  /* 3*16 = 48 像素宽 */

        x = (u16)(x + 48);
        draw_string(x, y, ":", COL_GREEN);
        x = (u16)(x + 8);
        draw_string(x, y, "1155211183", COL_GREEN);
    }

    /* --------- 第2行：侯卓昊 1155157303 --------- */
    {
        const unsigned char* partner[3];
        partner[0] = HZ16_HOU;
        partner[1] = HZ16_ZHUO;
        partner[2] = HZ16_HAO;

        x = 20;
        y = 170;
        showChinString(x, y, partner, 3, COL_YELLOW);

        x = (u16)(x + 48);
        draw_string(x, y, ":", COL_GREEN);
        x = (u16)(x + 8);
        draw_string(x, y, "1155157303", COL_GREEN);
    }

    /* 4. 底部提示文字（目前只是显示，不真正等按键） */
    draw_string(20, 80, "Press 0 to continue", COL_WHITE);

    {
        char best_buf[32];
        sprintf(best_buf, "Your highest score is %d", high_score);
        draw_string(20, 110, best_buf, COL_WHITE);
    }


}
