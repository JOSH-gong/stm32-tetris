#include "stm32f10x.h"
#include "tetris_title.h"
#include "tetris_board.h"
#include "tetris_logic.h"
#include "IERG3810_TFTLCD.h"
#include "ps2.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_LED.h"
#include <stdio.h>

/* 计时：TIM2 预分频到 10kHz，直接读取 CNT (0.1ms/tick, 16-bit wrap) */
static uint16_t NowTicks(void)
{
    return (uint16_t)(TIM2->CNT);
}

static int g_high_score = 0; /* 运行期间记录最高分 */

static uint32_t g_random_state = 123456789u;

static void Random_AddEntropy(uint32_t entropy)
{
    g_random_state ^= entropy + 0x9E3779B9u + (g_random_state << 6) + (g_random_state >> 2);
}

static uint32_t Random_Next(void)
{
    g_random_state = g_random_state * 1664525u + 1013904223u;
    return g_random_state;
}

/* 短暂“戳一下”PS/2，避免在耗时 LCD 操作期间漏掉边沿 */
static void PS2_Tickle(int times)
{
    int i;
    for (i = 0; i < times; ++i) {
        PS2_Poll();
    }
}

/* 简单的主程序：显示标题页，等小键盘 '0' (PS/2 make code 0x70) 按下，
   然后进入并绘制棋盘。此文件为可修改的 main.c，其它 User 下示例文件
   仅供参考，不应被改动。 */

/* 处理外部中断所需的占位心跳函数（有些库会引用） */
void heartbeat(void) {}

/* PS/2 处理已移到 Board/ps2.c - main 使用 PS2_Init() / PS2_GetLastMakeCode() */

/* 调试：在屏幕右下角显示最近一次扫描码（低8位十六进制；带 E 表示有 0xE0 扩展） */
static void Debug_ShowScancode(uint16_t code)
{
    char buf[12];
    uint8_t sc = (uint8_t)(code & 0xFF);
    int is_ext = (code & 0x100) ? 1 : 0;

    buf[0] = 'S'; buf[1] = 'C'; buf[2] = ':';
    buf[3] = "0123456789ABCDEF"[(sc >> 4) & 0xF];
    buf[4] = "0123456789ABCDEF"[sc & 0xF];
    buf[5] = is_ext ? 'E' : ' ';
    buf[6] = '\0';

    /* 位置可根据屏幕情况调整，这里放在右下角附近并顺便清空背景 */
    IERG3810_TFTLCD_FillRectangle(BLACK, 180, 60, 300, 16);
    IERG3810_TFTLCD_ShowString(180, 300, buf, WHITE);
}

int main(void)
{
    uint16_t code = 0; /* bit8 标记是否有 0xE0 扩展前缀 */
    uint8_t sc = 0;    /* 统一使用的扫描码临时变量，避免在语句后声明 */
    int score = 0;
    int elapsed_seconds = 0;
    uint16_t last_ticks = 0;
    uint32_t tick_accum = 0; /* 0.1ms ticks 累积 */

    /* 1) 显示初始标题页（这个函数在 Board/tetris_title.c 中实现） */
    TETRIS_ShowTitlePage(g_high_score);

    /* 2) 初始化 PS/2（用于小键盘输入） */
    PS2_Init();

    IERG3810_Buzzer_Init();
    IERG3810_LED_Init();

    /* 2.5) 初始化 TIM2 为 10kHz 基准（0.1ms/tick，无中断） */
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

        TIM2->PSC = 7200 - 1;   /* 72MHz /7200 = 10kHz -> 0.1ms per tick */
        TIM2->ARR = 0xFFFF;
        TIM2->EGR = TIM_EGR_UG;
        TIM2->CR1 = TIM_CR1_CEN;

        last_ticks = NowTicks();
        tick_accum = 0;
        Random_AddEntropy(((uint32_t)last_ticks << 16) ^ tick_accum);
    }

    /* 3) 等待小键盘 '0' (make code 0x70) 被按下，然后进入棋盘 */
    while (1) {
        /* 连续多次轮询 + 立刻取码，避免中途按键被覆盖 */
        int p;
        for (p = 0; p < 400; ++p) {
            PS2_Poll();
            code = PS2_GetLastMakeCode();
            Random_AddEntropy(((uint32_t)NowTicks() << 16) ^ code);
            if (code != 0) {
                sc = (uint8_t)(code & 0xFF); /* 忽略扩展标记，只用低 8 位比较 */
                Debug_ShowScancode(code);
                /* 小键盘 '0' 的 make code = 0x70 */
                if (sc == 0x70) {
game_restart:
                    /* 进入棋盘界面：先确保 LCD 初始化并清屏，关掉标题页 */
                    IERG3810_TFTLCD_Init(); /* 确保 LCD 已初始化 */
                    /* 清除整个屏幕，覆盖标题页 */
                    IERG3810_TFTLCD_FillRectangle(BLACK, 0, 240, 0, 320);
                    /* 再绘制棋盘区域 */

                    Tetris_ClearBoard();
                    Tetris_DrawBorder(WHITE);
                    /* 绘制并初始化侧边栏（分数/时间/等级选择） */
                    Tetris_DrawSidePanels();

                    score = 0;
                    elapsed_seconds = 0;
                    last_ticks = NowTicks();
                    tick_accum = 0;
                    Tetris_UpdateScore(score);
                    Tetris_UpdateTime(elapsed_seconds);
                    Tetris_UpdateLevel(1);
                    IERG3810_Buzzer_BeepStart();
                    IERG3810_LED_Flash_DS0(10000, 1);
                    IERG3810_LED_Flash_DS1(10000, 1);
                    IERG3810_Buzzer_BeepStart();

                        /* 在这里进入游戏主循环：实现方块下落机制（不含消除） */
                        {
                            /* 所有变量声明放在块顶部以兼容 C90 */
                        static uint8_t board[TETRIS_ROWS][TETRIS_COLS];
                        const int pieces_local[7][4][2] = {
                            /* I */ {{0,1},{1,1},{2,1},{3,1}},
                            /* J */ {{0,0},{0,1},{1,1},{2,1}},
                            /* L */ {{2,0},{0,1},{1,1},{2,1}},
                            /* O */ {{1,0},{2,0},{1,1},{2,1}},
                            /* S */ {{1,0},{2,0},{0,1},{1,1}},
                            /* T */ {{1,0},{0,1},{1,1},{2,1}},
                            /* Z */ {{0,0},{1,0},{1,1},{2,1}}
                        };
                        u16 fallback_colors_local[8] = {BLUE, GREEN, RED, YELLOW, 0x07FF, 0xF81F, WHITE, 0x07E0};
                        int cur_id, cur_x, cur_y;
                        int active_piece[4][2];      /* 当前活动方块的偏移（随旋转变化） */
                        int new_offsets[4][2];       /* 旋转时的临时偏移 */
                        int last_drawn_piece[4][2];  /* 上一次绘制的方块形状，用于擦除 */
                        int k, kk, kk2;
                        int m, can_move;
                        int prev_x, prev_y;
                        int last_drawn_x, last_drawn_y;
                        int tick = 0;
                        int dropInterval = 14;
                        int needs_redraw = 1; /* 只有位置/旋转变化时才重绘，减少无效绘制 */

                        /* 初始化棋盘数据 */
                        {
                            int ri, rj;
                            for (ri = 0; ri < TETRIS_ROWS; ++ri)
                                for (rj = 0; rj < TETRIS_COLS; ++rj)
                                    board[ri][rj] = 0;
                        }

                        /* 生成第一个方块 */
                        cur_id = (int)(Random_Next() % 7);
                        cur_x = (TETRIS_COLS / 2) - 2;
                        cur_y = 0;

                        /* 初始化活动方块偏移（复制基础形状） */
                        for (k = 0; k < 4; ++k) {
                            active_piece[k][0] = pieces_local[cur_id][k][0];
                            active_piece[k][1] = pieces_local[cur_id][k][1];
                            last_drawn_piece[k][0] = active_piece[k][0];
                            last_drawn_piece[k][1] = active_piece[k][1];
                        }

                        /* 先绘制一次静态界面元素（避免每帧重绘造成闪烁） */
                        Tetris_ClearBoard();
                        Tetris_DrawBorder(WHITE);
                        Tetris_DrawSidePanels();
                        Tetris_UpdateLevel(1);

                        prev_x = cur_x;
                        prev_y = cur_y;
                        last_drawn_x = cur_x;
                        last_drawn_y = cur_y;

                        /* 主下落循环：只擦除/绘制活动方块的位置，锁定格子直接写入 board 并绘制 */
                        while (1) {
                            /* 累计时间：TIM2 10kHz，10 tick=1ms，10000 tick=1s */
                            {
                                uint16_t now = NowTicks();
                                uint16_t delta = (uint16_t)(now - last_ticks); /* 处理计数溢出 */
                                last_ticks = now;
                                tick_accum += delta; /* 单位 0.1ms */
                            }
                            while (tick_accum >= 10000U) { /* 10000 *0.1ms = 1s */
                                tick_accum -= 10000U;
                                elapsed_seconds++;
                                Tetris_UpdateTime(elapsed_seconds);
                            }

                            /* 先处理输入，再决定是否重绘，避免绘制阻塞按键响应 */
                            {
                                int p_input;
                for (p_input = 0; p_input < 120; ++p_input) {
                    PS2_Poll();
                    code = PS2_GetLastMakeCode();
                    Random_AddEntropy(((uint32_t)NowTicks() << 16) ^ code);
                    if (code == 0) continue;
                    Debug_ShowScancode(code);
                                    sc = (uint8_t)(code & 0xFF); /* 低 8 位为扫描码，bit8 为扩展标记 */

                                    /* 左移：numpad 4 (0x6B) */
                                    if (sc == 0x6B) {
                                        can_move = 1;
                                        for (m = 0; m < 4; ++m) {
                                            int px = cur_x + active_piece[m][0] - 1;
                                            int py = cur_y + active_piece[m][1];
                                            if (px < 0 || px >= TETRIS_COLS || py < 0 || py >= TETRIS_ROWS) { can_move = 0; break; }
                                            if (board[py][px]) { can_move = 0; break; }
                                        }
                                        if (can_move) { cur_x -= 1; needs_redraw = 1; }
                                    }
                                    /* 右移：numpad 6 (0x74) */
                                    else if (sc == 0x74) {
                                        can_move = 1;
                                        for (m = 0; m < 4; ++m) {
                                            int px = cur_x + active_piece[m][0] + 1;
                                            int py = cur_y + active_piece[m][1];
                                            if (px < 0 || px >= TETRIS_COLS || py < 0 || py >= TETRIS_ROWS) { can_move = 0; break; }
                                            if (board[py][px]) { can_move = 0; break; }
                                        }
                                        if (can_move) { cur_x += 1; needs_redraw = 1; }
                                    }
                                    /* 软降：numpad 2 (0x72)，在不碰撞时向下加速一格 */
                                    else if (sc == 0x72) {
                                        int can_drop = 1;
                                        for (m = 0; m < 4; ++m) {
                                            int px = cur_x + active_piece[m][0];
                                            int py = cur_y + active_piece[m][1] + 1;
                                            if (px < 0 || px >= TETRIS_COLS || py < 0 || py >= TETRIS_ROWS) { can_drop = 0; break; }
                                            if (board[py][px]) { can_drop = 0; break; }
                                        }
                                        if (can_drop) { cur_y += 1; needs_redraw = 1; }
                                    }
                                    /* 旋转：用小键盘 8 (0x75)；方块 O (cur_id==3) 不旋转 */
                                    else if (sc == 0x75) {
                                        if (cur_id != 3) {
                                            int pivot_x = active_piece[1][0];
                                            int pivot_y = active_piece[1][1];
                                            int can_rotate = 1;
                                            for (m = 0; m < 4; ++m) {
                                                int rx = active_piece[m][0] - pivot_x;
                                                int ry = active_piece[m][1] - pivot_y;
                                                /* 顺时针旋转: (x,y) -> ( -y, x ) 相对于 pivot */
                                                int nx = -ry;
                                                int ny = rx;
                                                new_offsets[m][0] = pivot_x + nx;
                                                new_offsets[m][1] = pivot_y + ny;
                                                /* 检查新位置是否合法 */
                                                {
                                                    int px = cur_x + new_offsets[m][0];
                                                    int py = cur_y + new_offsets[m][1];
                                                    if (px < 0 || px >= TETRIS_COLS || py < 0 || py >= TETRIS_ROWS) { can_rotate = 0; break; }
                                                    if (board[py][px]) { can_rotate = 0; break; }
                                                }
                                            }
                                            if (can_rotate) {
                                                for (m = 0; m < 4; ++m) {
                                                    active_piece[m][0] = new_offsets[m][0];
                                                    active_piece[m][1] = new_offsets[m][1];
                                                }
                                                needs_redraw = 1;
                                            }
                                        }
                                    }
                                }
                            }

                            if (needs_redraw) {
                                /* 先擦除上一帧绘制的方块（使用 last_drawn_* 形状和位置） */
                                for (k = 0; k < 4; ++k) {
                                    int pxp = last_drawn_x + last_drawn_piece[k][0];
                                    int pyp = last_drawn_y + last_drawn_piece[k][1];
                                    if (pxp >= 0 && pxp < TETRIS_COLS && pyp >= 0 && pyp < TETRIS_ROWS) {
                                        if (board[pyp][pxp] == 0) {
                                            Tetris_DrawCell_topOrigin(pxp, pyp, BLACK);
                                            PS2_Tickle(2);
                                        }
                                    }
                                }

                                /* 擦除上一帧的活动方块（仅当对应格子未被固定时） */
                                for (k = 0; k < 4; ++k) {
                                    int pxp = prev_x + active_piece[k][0];
                                    int pyp = prev_y + active_piece[k][1];
                                    if (pxp >= 0 && pxp < TETRIS_COLS && pyp >= 0 && pyp < TETRIS_ROWS) {
                                        if (board[pyp][pxp] == 0) {
                                            Tetris_DrawCell_topOrigin(pxp, pyp, BLACK);
                                            PS2_Tickle(2);
                                        }
                                    }
                                }

                                /* 绘制当前活动方块 */
                                for (k = 0; k < 4; ++k) {
                                    int px, py;
                                    u16 col;
                                    px = cur_x + active_piece[k][0];
                                    py = cur_y + active_piece[k][1];
                                    if (px >= 0 && px < TETRIS_COLS && py >= 0 && py < TETRIS_ROWS) {
                                        col = fallback_colors_local[(cur_id + 1) % 8];
                                        Tetris_DrawCell_topOrigin(px, py, col);
                                        PS2_Tickle(2);
                                    }
                                }

                                prev_x = cur_x;
                                prev_y = cur_y;
                                last_drawn_x = cur_x;
                                last_drawn_y = cur_y;
                                for (k = 0; k < 4; ++k) {
                                    last_drawn_piece[k][0] = active_piece[k][0];
                                    last_drawn_piece[k][1] = active_piece[k][1];
                                }
                                needs_redraw = 0;
                            }

                            /* 基于等级的下落定时：等级越低下落越慢（间隔更大），等级越高间隔减小
                               这里使用线性映射：dropInterval = base - (level-1)*step
                               level 范围假设为 1..9，base 与 step 可调整以调节速度。 */
                            {
                                int base = 140;       /* 默认速度：较慢，便于调试 */
                                dropInterval = base;
                                if (dropInterval < 10) dropInterval = 10; /* 下限保护 */
                            }

                            /* 小延迟并计时，下落速度由 dropInterval 控制；顺便轮询键盘以防漏码 */
                            {
                                int d;
                                for (d = 0; d < 2000; ++d) { PS2_Poll(); __NOP(); }
                            }

                            /* 再次读取计时：TIM2 10kHz，10 tick=1ms，累计到 1 秒 */
                            {
                                uint16_t now = NowTicks();
                                uint16_t delta = (uint16_t)(now - last_ticks); /* wrap-safe 处理 */
                                last_ticks = now;
                                tick_accum += delta; /* 单位：0.1ms */
                            }
                            while (tick_accum >= 10000U) { /* 10000 x0.1ms = 1s */
                                tick_accum -= 10000U;
                                elapsed_seconds++;
                                Tetris_UpdateTime(elapsed_seconds);
                            }

                            tick++;
                            if (tick >= dropInterval) {
                                int will_collide = 0;
                                tick = 0;
                                /* 检查向下移动是否碰撞 */
                                for (kk = 0; kk < 4; ++kk) {
                                    int px = cur_x + active_piece[kk][0];
                                    int py = cur_y + active_piece[kk][1] + 1; /* 下移一格 */
                                    if (px < 0 || px >= TETRIS_COLS || py < 0 || py >= TETRIS_ROWS) { will_collide = 1; break; }
                                    if (board[py][px]) { will_collide = 1; break; }
                                }

                                if (!will_collide) {
                                    cur_y++;
                                    needs_redraw = 1;
                                } else {
                                    /* 固定方块并在 board 里标记 */
                                    for (kk = 0; kk < 4; ++kk) {
                                        int px, py;
                                        u16 col;
                                        px = cur_x + active_piece[kk][0];
                                        py = cur_y + active_piece[kk][1];
                                        if (px >= 0 && px < TETRIS_COLS && py >= 0 && py < TETRIS_ROWS) {
                                            board[py][px] = (uint8_t)(cur_id + 1);
                                            col = fallback_colors_local[(cur_id + 1) % 8];
                                            Tetris_DrawCell_topOrigin(px, py, col);
                                        }
                                    }

                                    /* 检查并清除满行，下移剩余方块 */
                                    {
                                        int lines = Tetris_ClearFullLines(board, fallback_colors_local);
                                        if (lines > 0) {
                                        score += lines * 100;
                                        Tetris_UpdateScore(score);
                                        IERG3810_Buzzer_BeepShort();
                                        IERG3810_LED_Flash_DS0(6000, 1);
                                        if (score > g_high_score) g_high_score = score;
                                        }
                                    }

                                    /* 生成新方块 */
                                cur_id = (int)(Random_Next() % 7);
                                    cur_x = (TETRIS_COLS / 2) - 2;
                                    cur_y = 0;

                                    /* 初始化活动方块偏移（复制基础形状） */
                                    for (k = 0; k < 4; ++k) {
                                        active_piece[k][0] = pieces_local[cur_id][k][0];
                                        active_piece[k][1] = pieces_local[cur_id][k][1];
                                    }

                                    /* 检查生成后是否立刻碰撞 -> 游戏结束 */
                                    {
                                        int collide_new = 0;
                                        for (kk2 = 0; kk2 < 4; ++kk2) {
                                            int px = cur_x + active_piece[kk2][0];
                                            int py = cur_y + active_piece[kk2][1];
                                            if (px < 0 || px >= TETRIS_COLS || py < 0 || py >= TETRIS_ROWS) { collide_new = 1; break; }
                                            if (board[py][px]) { collide_new = 1; break; }
                                        }
                                        if (collide_new) {
                                            /* 显示 Game Over 页面并传入最终分数与时间 */
                                            IERG3810_Buzzer_BeepGameOver();
                                            IERG3810_LED_Flash_DS1(15000, 2);
                                            Tetris_ShowGameOverPage(score, elapsed_seconds, g_high_score);
                                            goto game_restart;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* 不会到达 */
}
