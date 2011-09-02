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

    printf("===== [ CONSOLE TXTBUF: %d %d, total %d ] =============\n",
           con_tb.tb_head,
           con_tb.tb_tail,
           con_tb.tb_tail - con_tb.tb_head);

    for (ii = 0; tb_strget(&con_tb, ii, con_str, sizeof(con_str)); ii++)
    {
        fputs(con_str, stdout);
        slen += strlen(con_str);
    }

    printf("===== [ CONSOLE TOTAL: Strlen=%d, numlines=%d ] =======\n", slen, ii);

    fflush(stdout);
}
