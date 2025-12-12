#include "IERG3810_TFTLCD.h"
#include "ps2.h"
#include <stdio.h>

#define PANEL_W 36
#define LEFT_PANEL_X 4
#define RIGHT_PANEL_X (BOARD_X + BOARD_W + 4)
#define PANEL_MARGIN 4

/* 面板内文本布局（x 和 y 位置） */
#define LEFT_TEXT_X (LEFT_PANEL_X + PANEL_MARGIN)
#define RIGHT_TEXT_X (RIGHT_PANEL_X + PANEL_MARGIN)

/* Y 位置：y 越小越靠屏幕上方 */
#define LEFT_SCORE_LABEL_Y 40
#define LEFT_SCORE_VALUE_Y 60
#define LEFT_TIME_LABEL_Y 100
#define LEFT_TIME_VALUE_Y 120

#define RIGHT_LEVEL_LABEL_Y 40
#define RIGHT_LEVEL_VALUE_Y 60

/* 布局常量 */
#define TETRIS_COLS 10
#define TETRIS_ROWS 20
#define CELL 16
#define BOARD_X 40    /* 左下角 x 像素 */
#define BOARD_Y 0     /* 左下角 y 像素 */

#define BOARD_W (TETRIS_COLS * CELL)
#define BOARD_H (TETRIS_ROWS * CELL)

/* 清空棋盘背景为黑 */
void Tetris_ClearBoard(void)
{
    IERG3810_TFTLCD_FillRectangle(BLACK, BOARD_X, BOARD_W, BOARD_Y, BOARD_H);
}

/* 在棋盘上绘制单个格子（row 从底部 0 开始计） */
void Tetris_DrawCell_bottomOrigin(int col, int row, u16 color)
{
    u16 x;
    u16 y;

    if (col < 0 || col >= TETRIS_COLS || row < 0 || row >= TETRIS_ROWS) return;

    x = (u16)(BOARD_X + col * CELL);
    y = (u16)(BOARD_Y + row * CELL);
    IERG3810_TFTLCD_FillRectangle(color, x, CELL, y, CELL);
}

/* 如果逻辑上把 row 0 当作顶部，可用此函数转换后再绘制 */
void Tetris_DrawCell_topOrigin(int col, int row, u16 color)
{
    int flipped_row;

    if (col < 0 || col >= TETRIS_COLS || row < 0 || row >= TETRIS_ROWS) return;

    flipped_row = TETRIS_ROWS - 1 - row;
    Tetris_DrawCell_bottomOrigin(col, flipped_row, color);
}

/* 绘制细网格（可选，绘制网格线会多次写像素） */
void Tetris_DrawGrid(u16 lineColor)
{
    int i;

    /* 竖线 */
    for (i = 0; i <= TETRIS_COLS; i++)
    {
        u16 x = (u16)(BOARD_X + i * CELL);
        /* 宽度 1 像素，高度 BOARD_H */
        IERG3810_TFTLCD_FillRectangle(lineColor, x, 1, BOARD_Y, BOARD_H);
    }

    /* 横线 */
    for (i = 0; i <= TETRIS_ROWS; i++)
    {
        u16 y = (u16)(BOARD_Y + i * CELL);
        IERG3810_TFTLCD_FillRectangle(lineColor, BOARD_X, BOARD_W, y, 1);
    }
}

/* 绘制粗边框（例如 2 像素） */
void Tetris_DrawBorder(u16 borderColor)
{
    int left_x = BOARD_X - 2;
    int right_x = BOARD_X + BOARD_W;
    int top_y = BOARD_Y + BOARD_H;
    int bottom_y = BOARD_Y - 2;
    int width = BOARD_W + 4;
    int height = BOARD_H + 4;

    if (left_x < 0) left_x = 0;
    if (bottom_y < 0) bottom_y = 0;

    /* 左边 */
    IERG3810_TFTLCD_FillRectangle(borderColor, (u16)left_x, 2, (u16)bottom_y, (u16)height);
    /* 右边 */
    IERG3810_TFTLCD_FillRectangle(borderColor, (u16)right_x, 2, (u16)bottom_y, (u16)height);
    /* 底 */
    IERG3810_TFTLCD_FillRectangle(borderColor, (u16)left_x, (u16)width, (u16)bottom_y, 2);
    /* 顶 */
    IERG3810_TFTLCD_FillRectangle(borderColor, (u16)left_x, (u16)width, (u16)top_y, 2);
}

/* ---------------- 侧边面板：分数、时间、等级（不显示下一个方块） ---------------- */
void Tetris_DrawSidePanels(void)
{
    /* 绘制左右面板背景（全高度） */
    IERG3810_TFTLCD_FillRectangle(0x7BEF, LEFT_PANEL_X, PANEL_W, 0, 320); /* 浅灰色 */
    IERG3810_TFTLCD_FillRectangle(0x7BEF, RIGHT_PANEL_X, PANEL_W, 0, 320);

    /* 标签：在数值上方绘制标签，左对齐以保持整齐 */
    /* 使用缩短的标签以适应窄面板宽度 */
    IERG3810_TFTLCD_ShowString(LEFT_TEXT_X, LEFT_SCORE_LABEL_Y, "SCO", YELLOW);
    IERG3810_TFTLCD_ShowString(LEFT_TEXT_X, LEFT_TIME_LABEL_Y, "TIM", WHITE);

    /* 右侧面板：显示等级标签（已移除下一个方块预览） */
    IERG3810_TFTLCD_ShowString(RIGHT_TEXT_X, RIGHT_LEVEL_LABEL_Y, "LVL", WHITE);
}

/* 首页说明：在标题页上标出按键操作，便于玩家快速了解 */
void Tetris_ShowInstructionOverlay(void)
{
    IERG3810_TFTLCD_ShowString(20, 20, "INSTRUCTIONS", YELLOW);
    IERG3810_TFTLCD_ShowString(20, 40, "4: LEFT    6: RIGHT", WHITE);
    IERG3810_TFTLCD_ShowString(20, 60, "8: ROTATE  2: DROP", WHITE);
}

/* è®¡ç®å­ç¬¦ä¸²å¨å±å¹ä¸çå±ä¸­Xåæ ï¼ä¾¿äºå¤ç¨ */

/* ?????? 240 ????????? X ??????? */
static u16 Tetris_CenterX(const char *text)
{
    int len = 0;
    if (!text) return 0;
    while (text[len] != '\0') len++;
    if (len <= 0) return 0;
    {
        int sx = (240 - len * 8) / 2;
        if (sx < 0) sx = 0;
        return (u16)sx;
    }
}

/* ??? GAME OVER ???????????????????? 0 ?????? */
void Tetris_ShowGameOverPage(int score, int elapsed_seconds, int high_score)
{
    char buf[32];
    const char *title = "GAME OVER";
    const char *subtitle = "THANKS FOR PLAYING";
    const char *hint = "Press 0 to restart";
    const u16 topBar = 0x9000;
    const u16 topBarLight = 0xB800;
    const u16 panelShadow = 0x3186;
    const u16 panelInner = 0x0020;

    IERG3810_TFTLCD_FillRectangle(BLACK, 0, 240, 0, 320);
    IERG3810_TFTLCD_FillRectangle(topBar, 0, 240, 40, 70);
    IERG3810_TFTLCD_FillRectangle(topBarLight, 0, 240, 30, 20);
    IERG3810_TFTLCD_FillRectangle(topBarLight, 0, 240, 250, 20);

    IERG3810_TFTLCD_ShowString(Tetris_CenterX(title), 60, title, WHITE);
    IERG3810_TFTLCD_ShowString(Tetris_CenterX(subtitle), 90, subtitle, YELLOW);

    IERG3810_TFTLCD_FillRectangle(panelShadow, 20, 200, 120, 140);
    IERG3810_TFTLCD_FillRectangle(panelInner, 24, 192, 124, 132);
    {
        const u16 label_x = 32;
        const u16 value_x = 160;
        const u16 row_gap = 40;
        u16 y = 150;

        /* Row 1: label left, value right (same row) */
        IERG3810_TFTLCD_ShowString(label_x, y, "FINAL SCORE", RED);
        sprintf(buf, "%d", score);
        IERG3810_TFTLCD_ShowString(value_x, y, buf, WHITE);

        /* Row 2: SURVIVED label with elapsed time */
        y = (u16)(y + row_gap);
        IERG3810_TFTLCD_ShowString(label_x, y, "SURVIVED", BLUE);
        sprintf(buf, "%ds", elapsed_seconds);
        IERG3810_TFTLCD_ShowString(value_x, y, buf, WHITE);

        /* Row 3: best score */
        y = (u16)(y + row_gap);
        IERG3810_TFTLCD_ShowString(label_x, y, "BEST SCORE", YELLOW);
        sprintf(buf, "%d", high_score);
        IERG3810_TFTLCD_ShowString(value_x, y, buf, YELLOW);
    }

    IERG3810_TFTLCD_ShowString(Tetris_CenterX(hint), 290, hint, GREEN);

    while (1) {
        uint16_t code;
        uint8_t sc;
        PS2_Poll();
        code = PS2_GetLastMakeCode();
        if (code == 0) continue;
        sc = (uint8_t)(code & 0xFF);
        if (sc == 0x70) break; /* 输入 0 代表重新开始 */
    }
}

void Tetris_UpdateScore(int score)
{
    char buf[16];
    /* 在 SCORE 标签下绘制分数 */
    sprintf(buf, "%d", score);
    /* 清除先前的数值区域 */
    IERG3810_TFTLCD_FillRectangle(BLACK, (u16)(LEFT_TEXT_X), (u16)(PANEL_W - 2*PANEL_MARGIN), LEFT_SCORE_VALUE_Y, 16);
    IERG3810_TFTLCD_ShowString(LEFT_TEXT_X, LEFT_SCORE_VALUE_Y, buf, GREEN);
}

void Tetris_UpdateTime(int seconds)
{
    char buf[16];
    /* 显示简短时间：总秒数（适合窄面板）。 */
    sprintf(buf, "%d", seconds);
    /* 清除先前时间数值区域并绘制（最多3字符 -> 24px） */
    IERG3810_TFTLCD_FillRectangle(BLACK, (u16)(LEFT_TEXT_X), (u16)(PANEL_W - 2*PANEL_MARGIN), LEFT_TIME_VALUE_Y, 16);
    IERG3810_TFTLCD_ShowString(LEFT_TEXT_X, LEFT_TIME_VALUE_Y, buf, RED);
}

void Tetris_UpdateLevel(int level)
{
    char buf[8];
    sprintf(buf, "%d", level);
    /* 清除先前的等级数值区域 */
    IERG3810_TFTLCD_FillRectangle(BLACK, (u16)(RIGHT_TEXT_X), (u16)(PANEL_W - 2*PANEL_MARGIN), RIGHT_LEVEL_VALUE_Y, 16);
    /* 在 LVL 标签下左对齐绘制等级数值 */
    IERG3810_TFTLCD_ShowString(RIGHT_TEXT_X, RIGHT_LEVEL_VALUE_Y, buf, WHITE);
}

/* 交互选择：在右侧面板显示提示并闪烁等级值。使用 +/- 调整等级，按回车确认。
   回车的 make code 为主键 0x1C 或小键盘 0x5A。该函数为阻塞式，会轮询 PS/2。 */

