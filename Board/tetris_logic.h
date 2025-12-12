/* tetris_logic.h
 * Line-clear helper for Tetris board.
 */
#ifndef __TETRIS_LOGIC_H
#define __TETRIS_LOGIC_H

#include "tetris_board.h"
#include "stm32f10x.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 扫描已落下的格子，清除满行并下移上方方块。
 * board: 20x10 的格子数组，0=空，>0 表示颜色索引(1..7)。
 * colors: 颜色查找表，至少包含 8 个元素，对应 id%8。
 * 返回清除的行数。
 */
int Tetris_ClearFullLines(uint8_t board[TETRIS_ROWS][TETRIS_COLS], const u16 colors[8]);

#ifdef __cplusplus
}
#endif

#endif /* __TETRIS_LOGIC_H */
