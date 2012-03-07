/*
 * console.h - APme: Aion Automatic Abyss Point Tracker
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "console.h"
#include "txtbuf.h"

char con_buf[16384];
char con_str[1024];

struct txtbuf con_tb;

void con_init(void)
{
    tb_init(&con_tb, con_buf, sizeof(con_buf));
}

void con_printf(char *fmt, ...)
{
    va_list vargs;

#if 0
    va_start(vargs, fmt);
    vprintf(fmt, vargs);
    va_end(vargs);
#endif

    va_start(vargs, fmt);
    vsnprintf(con_str, sizeof(con_str), fmt, vargs);
    va_end(vargs);

    tb_strput(&con_tb, con_str);
}

void con_dump(void)
{
    int ii;

    size_t slen = 0;

    printf("===== [ CONSOLE TXTBUF: %ld %ld, total %ld ] =============\n",
           (long)con_tb.tb_head,
           (long)con_tb.tb_tail,
           (long)con_tb.tb_tail - (long)con_tb.tb_head);

    for (ii = 0; tb_strget(&con_tb, ii, con_str, sizeof(con_str)); ii++)
    {
        fputs(con_str, stdout);
        slen += strlen(con_str);
    }

    printf("===== [ CONSOLE TOTAL: Strlen=%ld, numlines=%d ] =======\n", (long)slen, ii);

    fflush(stdout);
}
