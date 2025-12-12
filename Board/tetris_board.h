/* tetris_board.h
 * Declarations for functions implemented in tetris_board.c
 */
#ifndef __TETRIS_BOARD_H
#define __TETRIS_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"

/* Board layout constants (match tetris_board.c) */
#define TETRIS_COLS 10
#define TETRIS_ROWS 20
#define CELL 16
#define BOARD_X 40
#define BOARD_Y 0
#define BOARD_W (TETRIS_COLS * CELL)
#define BOARD_H (TETRIS_ROWS * CELL)

/* Drawing API */
void Tetris_ClearBoard(void);
void Tetris_DrawCell_bottomOrigin(int col, int row, u16 color);
void Tetris_DrawCell_topOrigin(int col, int row, u16 color);
void Tetris_DrawGrid(u16 lineColor);
void Tetris_DrawBorder(u16 borderColor);
/* HUD / side panel API */
void Tetris_DrawSidePanels(void);
void Tetris_ShowGameOverPage(int score, int elapsed_seconds, int high_score); /* Game Over ?? */
void Tetris_ShowInstructionOverlay(void);
void Tetris_UpdateScore(int score);
void Tetris_UpdateTime(int seconds);
void Tetris_UpdateLevel(int level);
#ifdef __cplusplus
}
#endif

#endif /* __TETRIS_BOARD_H */
