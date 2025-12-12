#ifndef TETRIS_TITLE_H
#define TETRIS_TITLE_H

#include "stm32f10x.h"

/* 显示标题页（TETRIS + 中文名字 + CUID + 提示），并等待 K0(PA0) 按下 */
void TETRIS_ShowTitlePage(int high_score);

#endif
