/* tetris_logic.c - line clear and redraw helpers */
#include "tetris_logic.h"
#include "IERG3810_TFTLCD.h"
#include <string.h>

/* 闪烁满行一次，再清除并下移，最后重绘整个板面 */
int Tetris_ClearFullLines(uint8_t board[TETRIS_ROWS][TETRIS_COLS], const u16 colors[8])
{
    int r, c;
    int full[TETRIS_ROWS];
    int cleared = 0;

    /* 标记满行 */
    for (r = 0; r < TETRIS_ROWS; ++r) {
        int ok = 1;
        for (c = 0; c < TETRIS_COLS; ++c) {
            if (board[r][c] == 0) { ok = 0; break; }
        }
        full[r] = ok;
        if (ok) cleared++;
    }

    if (cleared == 0) return 0;

    /* 闪烁满行 */
    for (r = 0; r < TETRIS_ROWS; ++r) {
        if (!full[r]) continue;
        /* 用白色闪一次 */
        for (c = 0; c < TETRIS_COLS; ++c) {
            Tetris_DrawCell_topOrigin(c, r, WHITE);
        }
    }
    /* 简短延时 */
    Delay(20000);
    for (r = 0; r < TETRIS_ROWS; ++r) {
        if (!full[r]) continue;
        for (c = 0; c < TETRIS_COLS; ++c) {
            Tetris_DrawCell_topOrigin(c, r, BLACK);
        }
    }

    /* 清空满行 */
    for (r = 0; r < TETRIS_ROWS; ++r) {
        if (full[r]) {
            for (c = 0; c < TETRIS_COLS; ++c) board[r][c] = 0;
        }
    }

    /* 下移：从底部向上压缩非满行 */
    {
        int write_row = TETRIS_ROWS - 1;
        for (r = TETRIS_ROWS - 1; r >= 0; --r) {
            if (!full[r]) {
                if (write_row != r) {
                    for (c = 0; c < TETRIS_COLS; ++c) {
                        board[write_row][c] = board[r][c];
                    }
                }
                write_row--;
            }
        }
        /* 填充上方空行 */
        for (; write_row >= 0; --write_row) {
            for (c = 0; c < TETRIS_COLS; ++c) board[write_row][c] = 0;
        }
    }

    /* 重绘整个棋盘区域 */
    Tetris_ClearBoard();
    for (r = 0; r < TETRIS_ROWS; ++r) {
        for (c = 0; c < TETRIS_COLS; ++c) {
            uint8_t v = board[r][c];
            if (v) {
                u16 col = colors[(v) % 8];
                Tetris_DrawCell_topOrigin(c, r, col);
            }
        }
    }

    return cleared;
}
