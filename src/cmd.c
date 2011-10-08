#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>

#include "regeng.h"
#include "util.h"
#include "aion.h"
#include "items.h"
#include "console.h"
#include "help.h"
#include "version.h"

#define CMD_COMMAND_CHAR    '?'
#define CMD_CHATHIST_CHAR   '^'
#define CMD_SIZE            64
#define CMD_ARGC_MAX        32
#define CMD_DELIM           " ,"
#define CMD_TEXT_SZ         AION_CHAT_SZ

#define CMD_RETVAL_OK       "OK"
#define CMD_RETVAL_ERROR    "Error"
#define CMD_RETVAL_UNKNOWN  "Unknown"

static char cmd_retval[CMD_TEXT_SZ];

typedef bool cmd_func_t(int argc, char *argv[], char *txt);

static cmd_func_t cmd_func_help;
static cmd_func_t cmd_func_hello;
static cmd_func_t cmd_func_nameset;
static cmd_func_t cmd_func_ap_stats;
static cmd_func_t cmd_func_ap_loot;
static cmd_func_t cmd_func_ap_set;
static cmd_func_t cmd_func_ap_reset;
static cmd_func_t cmd_func_group_join;
static cmd_func_t cmd_func_group_leave;
static cmd_func_t cmd_func_elyos;
static cmd_func_t cmd_func_asmo;
static cmd_func_t cmd_func_relyos;
static cmd_func_t cmd_func_rasmo;
static cmd_func_t cmd_func_echo;
static cmd_func_t cmd_func_apcalc;
static cmd_func_t cmd_func_inv;
static cmd_func_t cmd_func_debug;

struct cmd_entry
{
    char        *cmd_command;
    cmd_func_t  *cmd_func;
};


struct cmd_entry cmd_list[] =
{
    {
        .cmd_command    = "help",
        .cmd_func       = cmd_func_help,
    },
    {
        .cmd_command    = "hello",
        .cmd_func       = cmd_func_hello,
    },
    {
        .cmd_command    = "name",
        .cmd_func       = cmd_func_nameset,
    },
    {
        .cmd_command    = "apstat",
        .cmd_func       = cmd_func_ap_stats,
    },
    {
        .cmd_command    = "aploot",
        .cmd_func       = cmd_func_ap_loot,
    },
    {
        .cmd_command    = "apset",
        .cmd_func       = cmd_func_ap_set,
    },
    {
        .cmd_command    = "apreset",
        .cmd_func       = cmd_func_ap_reset,
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
    {
        .cmd_command    = "relyos",
        .cmd_func       = cmd_func_relyos,
    },
    {
        .cmd_command    = "rasmo",
        .cmd_func       = cmd_func_rasmo,
    },
    {
        .cmd_command    = "echo",
        .cmd_func       = cmd_func_echo,
    },
    {
        .cmd_command    = "apcalc",
        .cmd_func       = cmd_func_apcalc,
    },
    {
        .cmd_command    = "inv",
        .cmd_func       = cmd_func_inv,
    },
    {
        .cmd_command    = "debug",
        .cmd_func       = cmd_func_debug,
    },
};

void cmd_retval_printf(char *fmt, ...)
{
    va_list vargs;

    va_start(vargs, fmt);
    vsnprintf(cmd_retval, sizeof(cmd_retval), fmt, vargs);
    va_end(vargs);

    /* Prevent recursions */
    if (cmd_retval[0] == '?') cmd_retval[0] = ' ';
}

void cmd_retval_set(const char *txt)
{
    util_strlcpy(cmd_retval, txt, sizeof(cmd_retval));

    /* Prevent recursions */
    if (cmd_retval[0] == '?') cmd_retval[0] = ' ';
}

bool cmd_func_help(int argc, char *argv[], char *txt)
{
    (void)txt;

    char help[1024];

    help_cmd(argc >= 2 ? argv[1] : NULL, help, sizeof(help));

    cmd_retval_set(help);

    return true;
}

bool cmd_func_hello(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    cmd_retval_printf("Hello, this is APme version %s.", APME_VERSION_STRING);

    return true;
}

bool cmd_func_nameset(int argc, char *argv[], char *txt)
{
    (void)txt;

    if (argc < 2) return false;

    aion_player_name_set(argv[1]);

    cmd_retval_printf("You are now known as %s.", argv[1]);

    return true;
}

bool cmd_func_ap_stats(int argc, char *argv[], char *txt)
{
    char buf[AION_CHAT_SZ];

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

bool cmd_func_ap_loot(int argc, char *argv[], char *txt)
{
    char buf[256];

    (void)argc;
    (void)argv;
    (void)txt;

    if (!aion_group_get_aplootrights(buf, sizeof(buf)))
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

bool cmd_func_ap_reset(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    aion_apvalue_reset();

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
    char tr_txt[CMD_TEXT_SZ];

    (void)argv;

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

bool cmd_func_rtranslate(char *argv[], char *txt, int langid)
{
    char tr_txt[CMD_TEXT_SZ];

    (void)argv;

    util_strlcpy(tr_txt, txt, sizeof(tr_txt));

    aion_rtranslate(tr_txt, langid);

    cmd_retval_set(tr_txt);

    return true;
}

bool cmd_func_relyos(int argc, char *argv[], char *txt)
{
    (void)argc;

    return cmd_func_rtranslate(argv, txt, LANG_ASMODIAN);
}

bool cmd_func_rasmo(int argc, char *argv[], char *txt)
{
    (void)argc;

    return cmd_func_rtranslate(argv, txt, LANG_ELYOS);
}
bool cmd_func_echo(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;

    cmd_retval_set(txt);

    return true;
}

bool cmd_func_apcalc(int argc, char *argv[], char *txt)
{
    static regex_t  cmd_apcalc_re1;
    static regex_t  cmd_apcalc_re2;
    static bool     cmd_apcalc_first = true;

    int             retval;
    regmatch_t      rematch[4];  /* We wont match more than 2 items */
    struct item     *relic_item;
    uint32_t        relic_num;

    uint32_t        ap_total = 0;

    (void)argv;
    (void)argc;

    /* Initialize the item regex */
    if (cmd_apcalc_first)
    {
        retval = regcomp(&cmd_apcalc_re1, "([0-9]*)x?\\[item:([0-9]+)[a-zA-Z0-9;]*\\]x?([0-9]*)", REG_EXTENDED);
        if (retval != 0)
        {
            con_printf("Error initializing apcalc\n");
            return false;
        }

        retval = regcomp(&cmd_apcalc_re2, "([0-9]*)x?<([A-Za-z ]+)>x?([0-9]*)", REG_EXTENDED);
        if (retval != 0)
        {
            con_printf("Error initializing apcalc\n");
            return false;
        }
        cmd_apcalc_first = false;
    }

    for (;;)
    {
        char relic_numstr[16];
        char relic_idstr[64];

        relic_numstr[0] = '\0';
        relic_idstr[0] = '\0';
        relic_num = 1;

        retval = -1;
        relic_item = NULL;

        if (retval != 0)
        {
            retval = regexec(&cmd_apcalc_re1,
                             txt,
                             sizeof(rematch) / sizeof(rematch[0]),
                             rematch,
                             0);
            if (retval == 0)
            {
                uint32_t relic_id;

                re_strlcpy(relic_idstr, txt, sizeof(relic_idstr), rematch[2]);
                if (relic_idstr[0] != '\0')
                {
                    relic_id = strtoul(relic_idstr, NULL, 0);
                }

                relic_id = strtoul(relic_idstr, NULL, 0);

                relic_item = item_find(relic_id);
            }
        }

        if (retval != 0)
        {
            retval = regexec(&cmd_apcalc_re2,
                             txt,
                             sizeof(rematch) / sizeof(rematch[0]),
                             rematch,
                             0);
            if (retval == 0)
            {
                re_strlcpy(relic_idstr, txt, sizeof(relic_idstr), rematch[2]);
                relic_item = item_find_name(relic_idstr);
            }
        }

        /* No more matchers found, bail out */
        if (retval != 0)
        {
            break;
        }

        if (re_strlen(rematch[1]) != 0)
        {
            re_strlcpy(relic_numstr, txt, sizeof(relic_numstr), rematch[1]);
        }

        if (re_strlen(rematch[3]) != 0)
        {
            re_strlcpy(relic_numstr, txt, sizeof(relic_numstr), rematch[3]);
        }

        con_printf("%s x '%s' = %p\n", relic_numstr, relic_idstr, relic_item);

        txt += re_strlen(rematch[0]);

        if (relic_numstr[0] != '\0')
        {
            relic_num = strtoul(relic_numstr, NULL, 0);
        }


        if (relic_item == NULL)continue;
        if (relic_num == 0) continue;

        ap_total += relic_item->item_ap * relic_num;
    }

    con_printf("AP TOTAL: %u\n", ap_total);
    cmd_retval_printf("%uAP", ap_total);

    return true;
}

/*
 * The ?inv (inventory full) function and sub-commands:
 *      - ?inv on
 *      - ?inv off
 *      - ?inv clear
 */
bool cmd_func_inv(int argc, char *argv[], char *txt)
{
    (void)txt;
    /*
     * The ?inv command has 3 subcommands, ON/OFF/CLEAR
     * For simplicity, lets check the only first two 
     * letters.
     * If there's no subcommand, display the current policy.
     */
    if (argc < 2)
    {
        /* Nothing to do, status is autmatically displayed below */
    }
    else if (strncasecmp(argv[1], "ON", 2) == 0)
    {
        aion_invfull_excl_set(true);
    }
    else if (strncasecmp(argv[1], "OF", 2) == 0)
    {
        aion_invfull_excl_set(false);
    }
    else if (strncasecmp(argv[1], "CL", 2) == 0)
    {
        aion_invfull_clear();

        cmd_retval_set(CMD_RETVAL_OK);

        return true;
    }

    /* Display status about current invfull policy */
    if (aion_invfull_excl_get())
    {
        cmd_retval_set(help_invfull_on);
    }
    else
    {
        cmd_retval_set(help_invfull_off);
    }

    return true;
}

bool cmd_func_debug(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    con_dump();

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

/*
 * Parse chat history format, N^^^^^Player
 */
bool cmd_chat_hist(int argc, char *argv[], char *player, size_t player_sz, int *msgnum)
{
    /* Regex for matching the !Player-X format  */
    static bool     cmd_chathist_first = true;
    static regex_t  cmd_chathist_re;

    int        retval;
    regmatch_t rematch[4];  /* We wont match more than 3 items */
    char       buf[256];


    if (cmd_chathist_first)
    {
        /* Compile the regex for matching the !Player-X format  */
        retval = regcomp(&cmd_chathist_re, "^(\\d*)(\\^+)(\\w+)$", REG_EXTENDED);
        if (retval != 0)
        {
            con_printf("Error initializing CMD subsystem\n");
            return false;
        }

        cmd_chathist_first = false;
    }


    /* This format always takes just two arguments */
    if (argc != 2) return false;

    /* Check if the first argument contains a ^ */
    if (strchr(argv[1], '^') == NULL) return false;

    /* Use regular expressions to parse the syntax */
    retval = regexec(&cmd_chathist_re,
                     argv[1],
                     sizeof(rematch) / sizeof(rematch[0]),
                     rematch,
                     0);
    if (retval != 0)
    {
        return false;
    }

    /* Calculate the message number */
    *msgnum = 0;
    
    /* Parse the number before the first ^ */
    re_strlcpy(buf, argv[1], sizeof(buf), rematch[1]);
    if (strlen(buf) > 0)
    {
        *msgnum += strtoul(buf, NULL, 0);
    }

    /* Count the number of ^ in this string */
    re_strlcpy(buf, argv[1], sizeof(buf), rematch[2]);
    if (strlen(buf) >= 1)
    {
        *msgnum += strlen(buf) - 1;
    }
    else
    {
        /* Something went wrong, this should be at least 1 */
        return false;
    }

    /* Finally copy the player name */
    re_strlcpy(player, argv[1], player_sz, rematch[3]);

    return true;
}

/*
 * Parse and execute a command 
 */ 
void cmd_exec(char *txt)
{
    char cmdbuf[CMD_SIZE];
    char cmdchat[CMD_TEXT_SZ];
    /* Parse the arguments */
    char cmdplayer[64];

    char *pcmdbuf;
    char *cmdtxt;
    int  argc;
    char *argv[CMD_ARGC_MAX];
    size_t ii;
    int msgnum;

    /* Extract the command */
    if (txt[0] != CMD_COMMAND_CHAR) return;
    txt++;

    /*
     * Extract the arguments, but copy txt first since
     * util_strsep() modifies the buffer 
     */
    util_strlcpy(cmdbuf, txt, sizeof(cmdbuf));
    
    pcmdbuf = cmdbuf;
    /* Build out an argc/argv like list of parameters */
    for (argc = 0; argc < CMD_ARGC_MAX; argc++)
    {
        do
        {
            argv[argc] = util_strsep(&pcmdbuf, CMD_DELIM);
        }
        while ((argv[argc] != NULL) && argv[argc][0] == '\0');

        if (argv[argc] == NULL) break;
    }

    /* Check if the user used the ?command !Player syntax */
    if (cmd_chat_hist(argc, argv, cmdplayer, sizeof(cmdplayer), &msgnum))
    {
        /* Use the player's last chat message as cmdtxt */
        if (aion_player_chat_get(cmdplayer, msgnum, cmdchat, sizeof(cmdchat)))
        {
            cmdtxt = cmdchat;
            /*
             * XXX: Security risk, remove all ? at the beginning, because
             * with commands like ?echo and ?elyos/asmo we might execute 
             * unwanted commands.
             */
            while (*cmdtxt == '?') cmdtxt++;

            con_printf("LAST CHAT: '%s'\n", cmdchat);
        }
        else
        {
            cmdtxt = "No chat";
        }
    }
    else
    {
        /* Nope, just pass the text after the command, but clear initial spaces */
        cmdtxt = txt;
        cmdtxt += strlen(argv[0]);
        while (*cmdtxt == ' ') cmdtxt++;
    }

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
    char txt[CMD_TEXT_SZ];

    if (clipboard_get_text(txt, sizeof(txt)))
    {
        cmd_exec(txt);
    }
}

