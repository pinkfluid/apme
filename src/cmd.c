/*
 * cmd.c - APme: Aion Automatic Abyss Point Tracker
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
 * APme command processing
 * 
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
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
#include "config.h"

/**
 * @defgroup cmd Command Processing and Chat History
 *
 * @brief This module processes APme commands and the chat history.
 *
 * This module periodically polls the clipboard in @ref cmd_poll(). If it detects
 * a command text (oen starting with '?', or CMD_COMMAND_CHAR) it assumes it's
 * an APme command text and it processes it.
 *
 * @{
 */
#define CMD_COMMAND_CHAR    '?'                 /**< Chat command prefix                        */
#define CMD_CHATHIST_CHAR   '^'                 /**< History command prefix                     */
#define CMD_SIZE            64                  /**< Maximum command size                       */
#define CMD_ARGC_MAX        32                  /**< Maximum number of arguments for a command  */
#define CMD_DELIM           " ,"                /**< delimiters between arguments               */
#define CMD_TEXT_SZ         AION_CHAT_SZ        /**< Total command text size                    */

#define CMD_RETVAL_OK       "OK"                /**< Default response on success                */
#define CMD_RETVAL_ERROR    "Error"             /**< Default response on error                  */
#define CMD_RETVAL_UNKNOWN  "Unknown command"   /**< Default response if command not found      */

/**
 * Whatever a command returns back to the user is written
 * into this static buffer
 */
static char cmd_retval[CMD_TEXT_SZ];

/**
 * This is the general command processing function format
 */

typedef bool cmd_func_t(int argc, char *argv[], char *txt);

static bool cmd_func_translate(char *txt, int langid);
static bool cmd_func_rtranslate(char *txt, int langid);

static char* cmd_sanitize(char *str);

static cmd_func_t cmd_func_help;            /**< Declaration of cmd_func_helpi()        */
static cmd_func_t cmd_func_hello;           /**< Declaration of cmd_func_hello()        */
static cmd_func_t cmd_func_nameset;         /**< Declaration of cmd_func_nameset()      */
static cmd_func_t cmd_func_ap_stats;        /**< Declaration of cmd_func_apstat()       */
static cmd_func_t cmd_func_ap_loot;         /**< Declaration of cmd_func_aploot()       */
static cmd_func_t cmd_func_ap_set;          /**< Declaration of cmd_ap_set()            */
static cmd_func_t cmd_func_ap_reset;        /**< Declaration of cmd_ap_reset()          */
static cmd_func_t cmd_func_ap_limit;        /**< Declaration of cmd_ap_limit()          */
static cmd_func_t cmd_func_ap_format;       /**< Declaration of cmd_ap_format()         */
static cmd_func_t cmd_func_group_add;       /**< Declaration of cmd_func_group_add()    */
static cmd_func_t cmd_func_group_del;       /**< Declaration of cmd_func_group_del()    */
static cmd_func_t cmd_func_group_leave;     /**< Declaration of cmd_func_group_leave()  */
static cmd_func_t cmd_func_elyos;           /**< Declaration of cmd_func_elyos()        */
static cmd_func_t cmd_func_asmo;            /**< Declaration of cmd_func_asmo()         */
static cmd_func_t cmd_func_relyos;          /**< Declaration of cmd_func_relyos()       */
static cmd_func_t cmd_func_rasmo;           /**< Declaration of cmd_func_rasmo()        */
static cmd_func_t cmd_func_echo;            /**< Declaration of cmd_func_echo()         */
static cmd_func_t cmd_func_apcalc;          /**< Declaration of cmd_func_apcalc()       */
static cmd_func_t cmd_func_inv;             /**< Declaration of cmd_func_inv()          */
static cmd_func_t cmd_func_dbgdump;         /**< Declaration of cmd_func_dbgdump()      */
static cmd_func_t cmd_func_dbgparse;        /**< Declaration of cmd_func_dbgparse()     */

/**
 * Chat command declaration structure
 *
 * @see cmd_list
 */
struct cmd_entry
{
    char        *cmd_command;       /**< Command name       */
    cmd_func_t  *cmd_func;          /**< Command function   */
};


/**
 * Global list of chat commands
 */
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
        .cmd_command    = "aplimit",
        .cmd_func       = cmd_func_ap_limit,
    },
    {
        .cmd_command    = "apformat",
        .cmd_func       = cmd_func_ap_format,
    },
    {
        .cmd_command    = "gradd",
        .cmd_func       = cmd_func_group_add,
    },
    {
        .cmd_command    = "grdel",
        .cmd_func       = cmd_func_group_del,
    },
    {
        .cmd_command    = "leave",
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
        .cmd_command    = "dbgdump",
        .cmd_func       = cmd_func_dbgdump,
    },
    {
        .cmd_command    = "dbgparse",
        .cmd_func       = cmd_func_dbgparse,
    }
};

/**
 * printf-like function for storing a command status strings to the return buffer
 *
 * @param[in]       fmt     printf-like format
 * @param[in]       ...     arguments
 */
void cmd_retval_printf(char *fmt, ...)
{
    va_list vargs;

    va_start(vargs, fmt);
    vsnprintf(cmd_retval, sizeof(cmd_retval), fmt, vargs);
    va_end(vargs);

    /* Prevent recursions */
    if (cmd_retval[0] == '?') cmd_retval[0] = ' ';
}

/**
 * Set the return string for a command
 *
 * param[in]        txt     Return string
 */
void cmd_retval_set(const char *txt)
{
    util_strlcpy(cmd_retval, txt, sizeof(cmd_retval));

    /* Prevent recursions */
    if (cmd_retval[0] == '?') cmd_retval[0] = ' ';
}

/**
 * This function implements the ?help command
 *
 * @param[in]       argc        Number of arguments in @p argv
 * @param[in]       argv        Command arguments:
 *                                  - argv[0] = Command name
 *                                  - argv[1] = Help topic
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_help(int argc, char *argv[], char *txt)
{
    (void)txt;

    char help[1024];

    help_cmd(argc >= 2 ? argv[1] : NULL, help, sizeof(help));

    cmd_retval_set(help);

    return true;
}

/**
 * This function implements the ?hello command, just displays the version number
 *
 * @param[in]       argc        Number of arguments in @p argv
 * @param[in]       argv        Command arguments:
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_hello(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    cmd_retval_printf("Hello, this is APme version %s.", APME_VERSION_STRING);

    return true;
}

/**
 * This function implements the ?name command, which sets the player's name
 *
 * @param[in]       argc        Number of arguments in @p argv
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1] = New name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_nameset(int argc, char *argv[], char *txt)
{
    (void)txt;

    if (argc < 2) return false;

    aion_player_name_set(argv[1]);

    /* Save to the configuration */
    cfg_set_string(CFG_SEC_APP, "name", argv[1]);

    cmd_retval_printf("You are now known as %s.", argv[1]);

    return true;
}

/**
 * This function implements the ?apstat command, which returns the current
 * AP statistics of the group
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_ap_stats(int argc, char *argv[], char *txt)
{
    char buf[AION_CHAT_SZ];

    (void)argc;
    (void)argv;
    (void)txt;

    if (!aion_aploot_stats(buf, sizeof(buf)))
    {
        return false;
    }

    cmd_retval_set(buf);

    return true;
}

/**
 * This function implements the ?aploot command, which returns the current
 * loot statistics
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_ap_loot(int argc, char *argv[], char *txt)
{
    char buf[256];

    (void)argc;
    (void)argv;
    (void)txt;

    if (!aion_aploot_rights(buf, sizeof(buf)))
    {
        return false;
    }

    cmd_retval_set(buf);

    return true;
}

/**
 * This function implements the ?apset command, sets the AP accumulated
 * by a player to a certain value
 *
 * @param[in]       argc        Number of arguments in @p argv
 * @param[in]       argv        Command arguments:
 *                                  - argv[0] = Command name
 *                                  - argv[1] = Player name
 *                                  - argv[2] = New AP value
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        On success
 * @retval          false       If argument format error
 */
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

/**
 * This function implements the ?apreset command, which resets
 * the AP statistics for all known players to 0
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_ap_reset(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    aion_apvalue_reset();

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

/**
 * This function implements the ?aplimit command, sets the upper bound
 * limit for the AP loot
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1] = New AP limit value
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        On success
 * @retval          false       If argument format error
 *
 * @see aion_aplimit_set
 */
bool cmd_func_ap_limit(int argc, char *argv[], char *txt)
{
    uint32_t apvalue;

    (void)txt;

    if (argc < 2)
    {
        return false;
    }

    apvalue = strtoul(argv[1], NULL, 0);

    aion_aplimit_set(apvalue);

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

/**
 * This function implements the ?apformat command, it sets
 * the aploot format
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1] = New aploot format
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        On success
 * @retval          false       If argument format error
 *
 * @see aion_aploot_fmt_set()
 */
bool cmd_func_ap_format(int argc, char *argv[], char *txt)
{
    (void)argv;

    if (argc < 2)
    {
        return false;
    }

    if (!aion_aploot_fmt_set(txt))
    {
        cmd_retval_set("Invalid format");
        return true;
    }

    cfg_set_string(CFG_SEC_APP, "apformat", txt);

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

/**
 * This function implements the ?gradd command, which manually adds a character
 * to the current group if it is not autodetected automatically
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1+] = Players to add to the group
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_group_add(int argc, char *argv[], char *txt)
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

/**
 * This function implements the ?grdeel command, which manually removes a character 
 * from the current group if it is not autodetected automatically
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1+] = Players to remove from the group
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_group_del(int argc, char *argv[], char *txt)
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

/**
 * This function implements the ?leave command. The player should use
 * this to leave the a group if it's not autodetected.
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1+] = Players to remove from the group
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_group_leave(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    aion_group_leave(AION_NAME_DEFAULT);

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}
/**
 * Generic translate functions, used by @ref cmd_func_elyos() 
 * and @ref cmd_func_asmo
 *
 * @param[in,out]   txt         Full chat line text with the command stripped
 * @param[in]       langid      Language ID
 * 
 * @retval          true        Always true at the moment
 *
 * @see aion_translate
 */
bool cmd_func_translate(char *txt, int langid)
{
    char tr_txt[CMD_TEXT_SZ];

    util_strlcpy(tr_txt, txt, sizeof(tr_txt));

    aion_translate(tr_txt, langid);

    cmd_retval_set(tr_txt);

    return true;
}

/**
 * This function implements the ?elyos translator command
 * This is used by Elyos to talk to Asmodians
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @return Returns the error code from @ref cmd_func_translate()
 */
bool cmd_func_elyos(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;

    return cmd_func_translate(txt, LANG_ELYOS);
}

/**
 * This function implements the ?asmo translator command
 * This is used by Elyos to talk to Asmodians
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @return Returns the error code from @ref cmd_func_translate()
 */
bool cmd_func_asmo(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;

    return cmd_func_translate(txt, LANG_ASMODIAN);
}

/**
 * Generic reverse translate function, used by @ref cmd_func_relyos() 
 * and @ref cmd_func_rasmo
 *
 * This function is the exact opposite of @ref cmd_func_translate()
 *
 * @param[in,out]   txt         Full chat line text with the command stripped
 * @param[in]       langid      Language ID
 * 
 * @retval          true        Always true at the moment
 *
 * @see aion_rtranslate
 */
bool cmd_func_rtranslate(char *txt, int langid)
{
    char tr_txt[CMD_TEXT_SZ];

    util_strlcpy(tr_txt, txt, sizeof(tr_txt));

    aion_rtranslate(tr_txt, langid);

    cmd_retval_set(tr_txt);

    return true;
}

/**
 * This function implements the ?relyos translator command
 *
 * This function reverses the text translated by ?elyos back
 * to the original text (if you want to see what other
 * Asmodians are saying to Elyos)
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @return Returns the error code from @ref cmd_func_rtranslate()
 */
bool cmd_func_relyos(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;

    return cmd_func_rtranslate(txt, LANG_ASMODIAN);
}

/**
 * This function implements the ?rasmo translator command
 *
 * This function reverses the text translated by ?asmo back
 * to the original text (if you want to see what other
 * Elyos are saying to Asmodians)
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @return Returns the error code from @ref cmd_func_rtranslate()
 */
bool cmd_func_rasmo(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;

    return cmd_func_rtranslate(txt, LANG_ELYOS);
}

/**
 * This function implements the ?echo command
 *
 * This function just echoes back whatever text it received as argument
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 *
 * @note This is mainly useful for retrieving the chathistory
 */
bool cmd_func_echo(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;

    cmd_retval_set(txt);

    return true;
}

/**
 * This function implements the ?apcalc function
 *
 * This is used for calculating the values of relics.
 * For example, it can calculate how much AP is 
 * 5x&lt;Major Ancient Crown&gt;, or &lt;Major Ancient Crown&gt;x5
 *
 * @note
 * This function is  quite a complex function and rarely ever used.
 * This was somehow made obosolete by the relic appriser, so 
 * currently it is a candidate for removal.
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        true on success
 * @retval          false       On error
 */
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

/**
 * This function implements the ?inv command for setting the 
 * "inventory full" policy handling
 *
 * It supports arguments:
 *  - ?inv off: turn inventory full policy enforcment off
 *  - ?inv on: Turn inventory full policy enforcment on
 *  - ?inv clear: Clear the inventory full flags for all players
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1] = ?inv sub-command
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
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

/**
 * Implements the ?dbgdump function, which dumps the console to stdout
 *
 * This is a bit awkward to use so maybe somebody can improve this.
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        Always returns true
 */
bool cmd_func_dbgdump(int argc, char *argv[], char *txt)
{
    (void)argc;
    (void)argv;
    (void)txt;

    con_dump();

    cmd_retval_set(CMD_RETVAL_OK);

    return true;
}

/**
 * Implements the ?dbgparse debugging command
 *
 * It parses a piece of chatlog file from a file, which is useful
 * for debugging 
 *
 * @param[in]       argc        Number of arguments
 * @param[in]       argv        Command arguments
 *                                  - argv[0] = Command name
 *                                  - argv[1] = Path to chatlog file
 * @param[in]       txt         Full chat line text with the command stripped
 *
 * @retval          true        True on error
 * @retval          false       If parsing of the file fails
 */
bool cmd_func_dbgparse(int argc, char *argv[], char *txt)
{
    (void)txt;

    if (argc < 2)
    {
        return false;
    }

    if (!chatlog_readfile(argv[1]))
    {
        return false;
    }

    /* Nothing for now */
    cmd_retval_set(CMD_RETVAL_OK);
    return true;
}

/**
 * This functions scans the command arguments (argc,argv) and returns true if 
 * it contains a chatlog history command in the format of [N]^+NAME
 *
 * In case a chat history command is matched, it returns the name of the player
 * to retrieve the chat history and the message number, where 0 is the most recent
 * message
 *
 * @param[in]       argc        Number of elements in argv
 * @param[in]       argv        Argument array
 * @param[out]      player      Requested chat history player name
 * @param[in]       player_sz   Size of the buffer pointed to by @p player
 * @param[out]      msgnum      The index of the chat history line requested (0 being most recent)
 *
 * @retval          true        If argc/argv contain a valid chathistory command
 * @retval          false       Otherwise
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

/**
 * Parses the text in @p txt and if it starts with a '?' (CMD_COMMAND_CHAR)
 * string it processes it as an APme command
 *
 * All the command processing starts here.
 *
 * @param[in]       txt     Text to process (usually from the clipboard)
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

    aion_clipboard_set(cmd_retval);
}

/**
 * Sanitize the string @p str:
 *  - Remove any leading spaces and newlines
 *  - Use only the first line if it's a multi-line string
 *  - Remove any trailing spaces and newlines
 */
char* cmd_sanitize(char *str)
{
    char *nl;

    /* Skip blanks at the beginning of the line */
    while (strchr(" \r\n\t", *str) != 0)
    {
        str++;
    }

    /* If this is a multiline string, use just 1 line */
    nl = strchr("\r\n", *str);
    if (nl != NULL) 
    {
        *nl = '\0';
    }

    /* Clear any extra blanks at the end of the line */
    util_chomp(str);

    return str;
}

/**
 * Poll the clipboard, if we get some text, pass it to @ref cmd_exec().
 */
void cmd_poll(void)
{
    char txt[CMD_TEXT_SZ];

    if (clipboard_get_text(txt, sizeof(txt)))
    {
        cmd_exec(cmd_sanitize(txt));
    }
}


/**
 * @}
 */
