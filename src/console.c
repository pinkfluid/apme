#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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

    va_start(vargs, fmt);
    vsnprintf(con_str, sizeof(con_str), fmt, vargs);
    va_end(vargs);

    tb_strput(&con_tb, con_str);
}

void con_dump(void)
{
    int ii;

    printf("=========================================\n");
    for (ii = 0; tb_strget(&con_tb, ii, con_str, sizeof(con_str)); ii++)
    {
        fputs(con_str, stdout);
    }

    fflush(stdout);
}
