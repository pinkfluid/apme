#define CON_DEBUG
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

/**
 * @file
 * Debugging console for APme
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "console.h"
#include "txtbuf.h"
#include "util.h"

/**
 * @defgroup console APme Debugging Console
 * 
 * @brief APme debugging console. Debug messages are logged here.
 *
 * @{
 */
#define CON_STR_SZ  1024            /**< Maximum size of a con_printf string            */

static char con_buf[16384];         /**< Debugging console buffer, used for @p con_tb   */
static char con_str[CON_STR_SZ];    /**< Console string                                 */
static uint32_t con_str_rep;        /**< Number of times the last message was repeated  */

struct txtbuf con_tb;               /**< Console textbuffer @see txtbuf                 */

/**
 * Initialize the APme console
 *
 * This just initializes the @p con_tb textbuffer
 *
 * @note Must be called before other con_* functions
 */
void con_init(void)
{
    con_str[0] = '\0';
    con_str_rep = 0;

    tb_init(&con_tb, con_buf, sizeof(con_buf));
}

/**
 * Logs a text to the console; it uses a printf-like format
 *
 * @param[in]       fmt     printf-like format
 * @param[in]       ...     Additional arguments
 */
void con_printf(char *fmt, ...)
{
    char curstr[CON_STR_SZ];
    va_list vargs;

    va_start(vargs, fmt);
    vsnprintf(curstr, sizeof(curstr), fmt, vargs);
    va_end(vargs);

    if (strcmp(curstr, con_str) == 0)
    {
        con_str_rep++;
        return;
    }

    if (con_str_rep > 0)
    {
        char rep_str[CON_STR_SZ];
        snprintf(rep_str, sizeof(rep_str), "Last message was repeated %d more time/s.\n", con_str_rep);
        con_str_rep = 0;

        tb_strput(&con_tb, rep_str);
#ifdef CON_DEBUG
        fputs(rep_str, stdout);
#endif
    }

    util_strlcpy(con_str, curstr, sizeof(con_str));

#ifdef CON_DEBUG
    fputs(con_str, stdout);
#endif
    tb_strput(&con_tb, con_str);
}

/**
 * Dumps the console to standard output
 */
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

/**
 * @}
 */

