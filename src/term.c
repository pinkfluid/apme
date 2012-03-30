/*
 * term.c - APme: Aion Automatic Abyss Point Tracker
 *
 * Copyright (C) 2012 Mitja Horvat <pinkfluid@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

/**
 * @file 
 *
 * OS independent simple terminal support functions
 *
 * @note The Windows Console under MinGW does not support VT100
 * codes, so we have to use the Win32 console functions
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#include <stdio.h>
#include "term.h"

/**
 * @defgroup term Terminal Support Module
 * @brief Simple OS-independent functions for handling terminals
 *
 * @note Currnetly supports only VT100 (Linux) and Windows consoles, but
 * the VT100 functions will not be documented in Doxygen.
 *
 * @{
 */

#if !defined(OS_MINGW)
/**
 * @cond TERM_VT100
 */
#define VT100_ESCAPE "\x1B"

void term_clear(void)
{
    printf(VT100_ESCAPE "c");
}

void term_setcolor(enum term_color color)
{
    int code;

    switch (color)
    {
        case TERM_COLOR_RESET:
            code = 0;
            break;

        case TERM_FG_GREEN:
            code = 32;
            break;

        case TERM_FG_YELLOW:
            code = 33;
            break;

        case TERM_FG_MAGENTA:
            code = 35;
            break;

        case TERM_FG_CYAN:
            code = 36;
            break;


        case TERM_BG_BLUE:
            code = 44;
            break;

        default:
            /* Unknown code, reset to default */
            code = 0;
            return;
    }

    printf(VT100_ESCAPE "[%dm", code);
}

/**
 * @endcond 
 */
#else

#include <windows.h>

/** Mask for the foreground color */
#define FOREGROUND_MASK (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED)
/** Mask for the background color */
#define BACKGROUND_MASK (BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED)

/** STDOUT Windows handle */
HANDLE win32con_handle = INVALID_HANDLE_VALUE;

/**
 * Initializes the win32 standard output handle,
 * @p win32con_handle
 */
static void term_get_handle(void)
{
    if (win32con_handle != INVALID_HANDLE_VALUE) return;

    win32con_handle = GetStdHandle(STD_OUTPUT_HANDLE);

}

/**
 * Clear the screen
 *
 * This basically fills the whole screen with a rectangle colored with
 * the background color
 */
void term_clear(void)
{
    CONSOLE_SCREEN_BUFFER_INFO duh;
    DWORD count;

    COORD coord = { 0, 0 };

    term_get_handle(); 

    GetConsoleScreenBufferInfo(win32con_handle, &duh);

    FillConsoleOutputCharacter(win32con_handle,
                               ' ',
                               duh.dwSize.X * duh.dwSize.Y,
                               coord,
                               &count);

    FillConsoleOutputAttribute(win32con_handle,
                               FOREGROUND_MASK,
                               duh.dwSize.X * duh.dwSize.Y,
                               coord,
                               &count);

    SetConsoleCursorPosition(win32con_handle, coord);
}

/**
 * Sets the color of the text, this affects only new printed 
 * text
 *
 * @param[in]       color       Color of the text
 */
void term_setcolor(enum term_color color)
{
    static WORD code = FOREGROUND_MASK;

    term_get_handle();

    switch (color)
    {
        case TERM_COLOR_RESET:
            code &= ~BACKGROUND_MASK;
            code |= FOREGROUND_MASK;
            break;

        case TERM_FG_GREEN:
            code &= ~FOREGROUND_MASK;
            code |= FOREGROUND_GREEN;
            break;

        case TERM_FG_YELLOW:
            code &= ~FOREGROUND_MASK;
            code |= FOREGROUND_RED | FOREGROUND_GREEN;
            break;

        case TERM_FG_MAGENTA:
            code &= ~FOREGROUND_MASK;
            code |= FOREGROUND_RED | FOREGROUND_BLUE;
            break;

        case TERM_FG_CYAN:
            code &= ~FOREGROUND_MASK;
            code |= FOREGROUND_BLUE | FOREGROUND_GREEN;
            break;

        case TERM_BG_BLUE:
            code &= ~BACKGROUND_MASK;
            code |= BACKGROUND_BLUE;
            break;

        default:
            /* Unknown code, do nothing */
            return;
    }

    SetConsoleTextAttribute(win32con_handle, code);
}

#endif

/**
 * @}
 */
