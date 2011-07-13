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

#define CMD_RETVAL_OK       "OK"
#define CMD_RETVAL_ERROR    "Error"
#define CMD_RETVAL_UNKNOWN  "Unknown"

static char cmd_retval[CMD_TEXT_SZ];

typedef bool cmd_func_t(int argc, char *argv[], char *txt);

static cmd_func_t cmd_func_hello;
static cmd_func_t cmd_func_ap_stats;
static cmd_func_t cmd_func_ap_roll;
static cmd_func_t cmd_func_ap_set;
static cmd_func_t cmd_func_group_join;
static cmd_func_t cmd_func_group_leave;
static cmd_func_t cmd_func_elyos;
static cmd_func_t cmd_func_asmo;

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
        .cmd_func       = cmd_func_ap_stats,
    },
    {
        .cmd_command    = "aproll",
        .cmd_func       = cmd_func_ap_roll,
    },
    {
        .cmd_command    = "apset",
        .cmd_func       = cmd_func_ap_set,
    },
    {
        .cmd_command    = "gradd",
        .cmd_func       = cmd_func_group_join,
    },
    {
        .cmd_command    = "grdel",
        .cmd_func       = cmd_func_group_leave,
    },
    {
        .cmd_command    = "elyos",
        .cmd_func       = cmd_func_elyos,
    },
    {
        .cmd_command    = "asmo",
        .cmd_func       = cmd_func_asmo,
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

bool cmd_func_ap_stats(int argc, char *argv[], char *txt)
{
    char buf[256];

    (void)argc;
    (void)argv;
    (void)txt;

    if (!aion_group_get_stats(buf, sizeof(buf)))
    {
        return false;
    }

    cmd_retval_set(buf);

    return true;
}

bool cmd_func_ap_roll(int argc, char *argv[], char *txt)
{
    char buf[256];

    (void)argc;
    (void)argv;
    (void)txt;

    if (!aion_group_get_aprollrights(buf, sizeof(buf)))
    {
        return false;
    }

    cmd_retval_set(buf);

    return true;
}

bool cmd_func_ap_set(int argc, char *argv[], char *txt)
{
    uint32_t apvalue;

    (void)txt;

    if (argc < 3)
    {
        return false;
    }

    apvalue = strtoul(argv[2], NULL, 0);

    if (!aion_group_apvalue_set(argv[1], apvalue))
    {
        return false;
    }

    cmd_retval_set(CMD_RETVAL_OK);
    return true;
}

bool cmd_func_group_join(int argc, char *argv[], char *txt)
{
    (void)txt;
    int ii;

    for (ii = 1; ii < argc; ii++)
    {
        aion_group_join(argv[ii]);
    }

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

bool cmd_func_group_leave(int argc, char *argv[], char *txt)
{
    (void)txt;
    int ii;

    for (ii = 1; ii < argc; ii++)
    {
        aion_group_leave(argv[ii]);
    }

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

bool cmd_func_translate(char *argv[], char *txt, int langid)
{
    char tr_txt[1024];

    util_strlcpy(tr_txt, txt, sizeof(tr_txt));

    aion_translate(tr_txt, langid);

    cmd_retval_set(tr_txt);

    return true;
}

bool cmd_func_elyos(int argc, char *argv[], char *txt)
{
    (void)argc;

    return cmd_func_translate(argv, txt, LANG_ELYOS);
}

bool cmd_func_asmo(int argc, char *argv[], char *txt)
{
    (void)argc;

    return cmd_func_translate(argv, txt, LANG_ASMODIAN);
}

/*
 * Parse and execute a command 
 */ 
void cmd_exec(char *txt)
{
    char cmdbuf[CMD_SIZE];
    char *pcmdbuf;
    char *cmdtxt;
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
    util_strlcpy(cmdbuf, txt, sizeof(cmdbuf));
    
    pcmdbuf = cmdbuf;
    /* Build out an argc/argv like list of parameters */
    for (argc = 0; argc < CMD_ARGC_MAX; argc++)
    {
        do
        {
            argv[argc] = strsep(&pcmdbuf, CMD_DELIM);
        }
        while ((argv[argc] != NULL) && argv[argc][0] == '\0');

        if (argv[argc] == NULL) break;
    }

    /* For cmdtxt, skip the command name and whitespaces */
    cmdtxt = txt;
    cmdtxt += strlen(argv[0]);
    while (*cmdtxt == ' ') cmdtxt++;

    cmd_retval_set(CMD_RETVAL_UNKNOWN);

    for (ii = 0; ii < sizeof(cmd_list) / sizeof(cmd_list[0]); ii++)
    {
        if (strcasecmp(cmd_list[ii].cmd_command, argv[0]) == 0)
        {
            if (!cmd_list[ii].cmd_func(argc, argv, cmdtxt))
            {
                cmd_retval_set(CMD_RETVAL_ERROR);
            }
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

