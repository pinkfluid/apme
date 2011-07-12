#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "util.h"
#include "aion.h"

#define CMD_CHAR            '?'
#define CMD_SIZE            64
#define CMD_ARGC_MAX        32
#define CMD_DELIM           " ,"
#define CMD_TEXT_SZ         1024

#define CMD_RETVAL_DEFAULT  "Unknown command"
#define CMD_RETVAL_UNKNOWN  "Unknown func"
#define CMD_RETVAL_PARAM    "Parameter error"

static char cmd_retval[CMD_TEXT_SZ];

typedef bool cmd_func_t(int argc, char *argv[], char *txt);

static cmd_func_t cmd_func_hello;
static cmd_func_t cmd_func_group_stats;

struct cmd_entry
{
    char        *cmd_command;
    cmd_func_t  *cmd_func;
};


struct cmd_entry cmd_list[] =
{
    {
        .cmd_command    = "hello",
        .cmd_func       = cmd_func_hello,
    },
    {
        .cmd_command    = "apstat",
        .cmd_func       = cmd_func_group_stats,
    },
};


void cmd_retval_printf(char *fmt, ...)
{
    va_list vargs;

    va_start(vargs, fmt);
    vsnprintf(cmd_retval, sizeof(cmd_retval), fmt, vargs);
    va_end(vargs);
}

void cmd_retval_set(char *txt)
{
    util_strlcpy(cmd_retval, txt, sizeof(cmd_retval));
}

bool cmd_func_hello(int argc, char *argv[], char *txt)
{
    cmd_retval_printf("Hello world: argc %d", argc);

    return true;
}

bool cmd_func_group_stats(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;
    char buf[128];

    if (!aion_group_get_stats(buf, sizeof(buf)))
    {
        cmd_retval_set(CMD_RETVAL_UNKNOWN);
        return false;
    }

    cmd_retval_set(buf);

    return true;
}

/*
 * Parse and execute a command 
 */ 
void cmd_exec(char *txt)
{
    char cmd[CMD_SIZE];
    char *pcmd;
    int  argc;
    char *argv[CMD_ARGC_MAX];
    int ii;

    /* Extract the command */
    if (txt[0] != CMD_CHAR) return;
    txt++;

    /*
     * Extract the arguments, but copy txt first since
     * strsep() modifies the buffer 
     */
    util_strlcpy(cmd, txt, sizeof(cmd));
    
    pcmd = cmd;
    /* Build out an argc/argv like list of parameters */
    for (argc = 0; argc < CMD_ARGC_MAX; argc++)
    {
        do
        {
            argv[argc] = strsep(&pcmd, CMD_DELIM);
        }
        while ((argv[argc] != NULL) && argv[argc][0] == '\0');

        if (argv[argc] == NULL) break;
    }

    cmd_retval_set(CMD_RETVAL_DEFAULT);

    for (ii = 0; ii < sizeof(cmd_list) / sizeof(cmd_list[0]); ii++)
    {
        if (strcasecmp(cmd_list[ii].cmd_command, argv[0]) == 0)
        {
            cmd_list[ii].cmd_func(argc, argv, cmd);
        }
    }

    clipboard_set_text(cmd_retval);
}

/*
 * Check if we have a valid command in the clipboard
 */
void cmd_poll(void)
{
    char txt[1024];

    if (clipboard_get_text(txt, sizeof(txt)))
    {
        cmd_exec(txt);
    }
}

