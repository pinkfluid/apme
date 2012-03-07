/*
 * term.h - APme: Aion Automatic Abyss Point Tracker
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
