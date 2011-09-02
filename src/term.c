#include <stdio.h>

#include "term.h"

#if !defined(OS_MINGW)

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

#else

/*
 * The Windows Console under MinGW does not support VT100 codes,
 * so we have to use the Win32 console functions
 */

#include <windows.h>

#define FOREGROUND_MASK (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED)
#define BACKGROUND_MASK (BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED)

HANDLE win32con_handle = INVALID_HANDLE_VALUE;

static void term_get_handle(void)
{
    if (win32con_handle != INVALID_HANDLE_VALUE) return;

    win32con_handle = GetStdHandle(STD_OUTPUT_HANDLE);

}

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

