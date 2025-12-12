# STM32 Tetris (Keil / STM32F10x)

A simple Tetris clone running on an STM32F10x microcontroller board, using a PS/2 keyboard for input and a TFT LCD for display.  
The project is developed and built in Keil µVision, based on the STM32F10x Standard Peripheral Library.

> Educational project for practicing embedded C, timer-based game loops, PS/2 polling, and TFT LCD graphics on STM32.

---

## Features

- Classic Tetris gameplay:
  - Left / right movement
  - Soft drop
  - Rotation (except O piece)
  - Line clearing and score accumulation
  - Game over detection and restart
- PS/2 keyboard input (numeric keypad)
- TFT LCD 240×320 graphical output
- Timer-based game loop using TIM2 (10 kHz, 0.1 ms tick)
- Simple pseudo-random piece generator (LCG)
- On-screen debug display of last PS/2 scancode

---

## Controls

All controls use the **numeric keypad** on the PS/2 keyboard:

- `0`  
  - Start game from title page  
- `4`  
  - Move active piece **left**  
- `6`  
  - Move active piece **right**  
- `2`  
  - **Soft drop**: move active piece down by one row if no collision  
- `8`  
  - **Rotate** the active piece clockwise  
  - Note: the `O` piece (square) does not rotate

When the board is filled and a new piece cannot be placed, the game shows a **Game Over** page with final score and elapsed time, then restarts from the title screen.

---

## Hardware Requirements

This project targets a typical STM32F10x development board, for example:

- MCU: STM32F10x series (e.g. STM32F103)
- TFT LCD:
  - 240×320 RGB TFT panel
  - Controlled via the `IERG3810_TFTLCD` driver
- PS/2 keyboard:
  - Connected to GPIO pins configured for PS/2 clock and data
  - Polled in software (no dedicated PS/2 hardware module)
- ST-LINK / J-LINK (or equivalent) for programming and debugging

You may need to adjust pin assignments and initialization code in the board support files to match your specific hardware.

---

## Project Structure

The exact layout may vary by Keil project, but the main modules are:

- `main.c`  
  - Entry point  
  - Shows title page  
  - Initializes PS/2 and TIM2 (10 kHz tick)  
  - Starts the main Tetris game loop
- `tetris_title.*`  
  - Functions for drawing the title page and game over page  
- `tetris_board.*`  
  - Board representation  
  - Cell drawing helpers and border rendering  
- `tetris_logic.*`  
  - Line clearing, score update, time/level display  
- `IERG3810_TFTLCD.*`  
  - TFT LCD initialization and drawing primitives  
- `ps2.*`  
  - PS/2 polling and scancode handling  
  - `PS2_Init()` / `PS2_Poll()` / `PS2_GetLastMakeCode()`

Supporting libraries (CMSIS, STM32F10x StdPeriph, startup files, etc.) are provided as part of the Keil project.

---

## Timing & Game Loop Design

- **TIM2** is configured to run at **10 kHz**:
  - Prescaler: `PSC = 7200 - 1` at 72 MHz system clock  
  - 1 tick = 0.1 ms  
- `NowTicks()` reads `TIM2->CNT` to obtain the current tick count.
- A 32-bit accumulator (`tick_accum`) converts ticks to **seconds**:
  - 10,000 ticks × 0.1 ms = 1 second
- Elapsed time is displayed on the side panel via `Tetris_UpdateTime()`.

The active Tetris piece is updated in a loop that:

1. Polls PS/2 input multiple times to avoid missing key events.
2. Updates position/rotation flags based on valid moves.
3. Computes when to **drop** the piece based on `dropInterval` (game speed).
4. Redraws only when needed (`needs_redraw`) to reduce flicker:
   - Erase old piece cells
   - Draw new piece cells
5. When collision is detected:
   - Lock the piece into `board[][]`
   - Clear filled lines
   - Spawn a new piece
   - If the new piece collides immediately, trigger **Game Over**.

---

## Build & Flash Instructions

1. **Open the Keil project**

   - Launch Keil µVision.
   - Open the `.uvproj` / `.uvprojx` file included in this repository.

2. **Check device configuration**

   - Target device: select an STM32F10x MCU matching your board (e.g. STM32F103).
   - Ensure the correct startup file and system clock configuration are used.

3. **Configure include paths / libraries**

   - CMSIS headers for STM32F10x
   - STM32F10x Standard Peripheral Library (if used)
   - Project-specific headers: `tetris_title.h`, `tetris_board.h`, `tetris_logic.h`, `IERG3810_TFTLCD.h`, `ps2.h`, etc.

4. **Build the project**

   - Click **Build** in Keil.
   - Fix any path / device / library issues if the build fails.

5. **Download to board**

   - Connect ST-LINK / J-LINK to the STM32 board.
   - Configure the debugger in Keil.
   - Click **Download** to flash the firmware.
   - Reset the board; the Tetris title page should appear on the TFT LCD.

---

## Debugging PS/2 Input

The helper function `Debug_ShowScancode()` displays the **last make code** on the TFT:

- Format: `SC:XYE`  
  - `XY` = last scancode in hex (low 8 bits)  
  - `E` = `'E'` if there was a `0xE0` extended prefix, otherwise space

This is useful when:

- Verifying which scancode a given key produces.
- Mapping PS/2 keys to game controls.
- Debugging wiring or PS/2 timing issues.

---

## Known Limitations / Future Work

- Level selection is currently hard-coded to **level 1** (no interactive menu).
- No "hard drop" or hold piece functionality.
- Simple collision and rotation logic (no wall kicks, etc.).
- Game speed vs. level uses a linear mapping; can be tuned for better gameplay.
- PS/2 handling is polling-based and may be refined with interrupts or buffering.

Possible extensions:

- Implement proper **level selection** on the title page.
- Add **hard drop** and **pause** keys.
- Add **high-score saving** in Flash.
- Improve rotation rules to be closer to modern Tetris guidelines.

---

## License

You can choose a license that fits your needs.  
Example (MIT License):

```text
MIT License

Copyright (c) 2025 <Your Name>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
...
