#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

enum term_color
{
    TERM_COLOR_RESET = 0,

    TERM_FG_GREEN = 10,
    TERM_FG_YELLOW = 11,
    TERM_FG_MAGENTA = 12,
    TERM_FG_CYAN = 13,

    TERM_BG_BLUE = 20,
};

extern void term_clear(void);
extern void term_setcolor(enum term_color color);

#endif /* TERM_H_INCLUDED */
