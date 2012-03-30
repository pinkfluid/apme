/*
 * help.c - APme: Aion Automatic Abyss Point Tracker
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
 *
 * Help and Strings
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "help.h"
#include "util.h"
#include "console.h"

/**
 * @defgroup help Help and Related Stuff
 * 
 * @brief Help files, help text....
 *
 * @{
 */

/** Structure that defines the help for the APme commands */
struct help_entry
{
    char *help_cmd;         /**< The help command or topic      */
    char *help_usage;       /**< Short usage help               */
    char *help_text;        /**< Full help text                 */
};

/** Chatlog warning text */
const char *help_chatlog_warning =
"!!!!! WARNING!!!!!! !!!!! WARNING!!!!!! !!!!! WARNING!!!!!! \n"
"\n"
"USE THIS APPLICATION AT YOUR OWN RISK\n"
"\n"
"The chatlog feature is required for this application to work properly,\n"
"but it is currently disabled.\n"
"\n"
"This application can enable the chatlog feature for you, but keep in\n"
"mind that using it was never officially aproved by NCSoft.\n"
"\n"
"Most likely WILL NOT get banned, but please be aware that you are \n"
"walking on the gray area until there is an official answer by NCSoft.\n"
"\n"
"The legality status of this application falls in the same category\n"
"as the DPS meters, since the principles behind are the same (chatlog\n"
"parsing). The difference is only in that I was kind enough to warn you :)\n"
"\n";

/** Chatlog enabled help text */
const char *help_chatlog_enabled =
"The chatlog feature was enabled in AION.\n"
"You must restart the game client to apply the new settings.\n"
"\n"
"At any point in time, you can remove the SYSTEM.OVR file in\n"
"the client directory to disable this feature.\n"
"\n";

/** Error enabling chatlog */
const char *help_chatlog_enable_error =
"Error enabling the CHATLOG feature. Restart the application to try again.\n";

/** Main screen help */
const char *help_mainscreen = 
"Type \"?command <PARAMS>\", selecte the text and copy-paste.\nFor more info, please use the \"?help <TOPIC>\" command.\n";

/** Inventory full policy enabled text */
const char *help_invfull_on =
"ON: Users with full inventory will be temporarily excluded from the AP fair loot system.";

/** Inventory full policy disabled text */
const char *help_invfull_off = 
"OFF: Users with full inventory will be warned only.";

/**
 * This is the help used for the ?help command
 */
struct help_entry help_commands[] =
{
    {
        "name",
        "?name <NAME>",
        "Set your name to <NAME> instead of the default 'You'."
    },
    {
        "apstat",
        "?apstat",
        "Display current Abyss Points of the group acquired from relics."
    },
    {
        "aploot",
        "?aploot",
        "Show current abyss relics loot rights."
    },
    {
        "apreset",
        "?apreset",
        "For all players, reset their accumulated AP points to 0.",
    },
    {
        "apset",
        "?apset <PLAYER> <AP>",
        "Set the AP points of <PLAYER> to <AP>."
    },
    {
        "aplimit",
        "?aplimit <AP>",
        "Set the upper AP limit per player. Players exceeding the AP limit will not be eligible for loot. Loot is FFA when the whole group reaches the limit. A value of 0 means no limit (default).",
    },
    {
        "gradd",
        "?gradd <PLAYER>",
        "Add <PLAYER> to your group, in case it is not autodetected."
    },
    {
        "grdel",
        "?grdel <PLAYER>",
        "Remove <PLAYER> from your group, in case it is not autodetected. Removing youreslf (?grdel You) disbands the group."
    },
    {
        "elyos",
        "?elyos <TEXT>",
        "Translate <TEXT> to elyos/from asmodian."
    },
    {
        "asmo",
        "?asmo <TEXT>",
        "Translate <TEXT> to asmodian/from elyos.",
    },
    {
        "relyos",
        "?relyos <TEXT>",
        "Reverse of the ?elyos command.",
    },
    {
        "reylos",
        "?rasmo <TEXT>",
        "Revers of the ?asmo command.",
    },
    {
        "apcalc",
        "?apcalc <RELIC ID> or <RELIC_ID>xN",
        "Calculate value of relic, For example ?apcalc <Major Ancient Crown>x5"
    },
    {
        "echo",
        "?echo <TEXT>",
        "Echoes <TEXT> back. Useful for inspecting chat history. For example ?echo ^<PLAYER> or ?echo ^<PLAYER>-1"
    },
    {
        "chathist",
        "chathist -- ?command ^<PLAYER_NAME> or ?command ^<PLAYER_NAME>-N",
        "Execute ?command but replace '^<PLAYER_NAME>' with the last text <PLAYER_NAME> entered into chat. Add -1 for second last text, -2 for third last..."
    },
    {
        "inv",
        "?inv on/off/clear",
        "When a player's inventory is full: OFF display only warnings, ON temporarily exclude the player from fair AP loot. Use CLEAR to clear the inventory full status (if it was misdetected)",
    },
    {
        "hello",
        "?hello",
        "Display the version number"
    },
};


/**
 * Find help text for @p keyword
 *
 * @param[in]       keyword     Help topic
 * 
 * @return
 * This function returns a pointer to a @p help_entry structure or NULL if help not found
 *
 */
struct help_entry *help_find(char *keyword)
{
    size_t ii;

    for (ii = 0; ii < sizeof(help_commands) / sizeof(help_commands[0]); ii++)
    {
        if (strcasecmp(keyword, help_commands[ii].help_cmd) == 0)
        {
            return &help_commands[ii];
        }
    }

    return NULL;
}

/**
 * This function returns the help for command @p cmd into the
 * buffer pointed to by @p help
 *
 * @param[in]       cmd         Help topic/command to search the help for
 * @param[out]      help        Help string
 * @param[in]       help_sz     Size of the buffer pointed to by @p help
 *
 */
void help_cmd(char *cmd, char *help, size_t help_sz)
{
    struct help_entry *he;

    /* Reset string */
    if ((cmd == NULL) || (strlen(cmd) == 0))
    {
        help_usage(help, help_sz);
        return;
    }

    he = help_find(cmd);
    if (he == NULL)
    {
        util_strlcpy(help, "Unknown command '", help_sz);
        util_strlcat(help, cmd, help_sz);
        util_strlcat(help, "'. Use ?help to get a list of all commands.", help_sz);
        return;
    }

    util_strlcpy(help, "Help: ", help_sz);
    util_strlcat(help, he->help_usage, help_sz);
    util_strlcat(help, " -- ", help_sz);
    util_strlcat(help, he->help_text, help_sz);
}

/**
 * Generate a list of all topics available and store it to the @p help
 * variable
 *
 * @param[out]      help        Variable pointing to the buffer where to save the help text
 * @param[in]       help_sz     Space available for the help text in bytes
 */
void help_usage(char *help, size_t help_sz)
{
    size_t ii;
    size_t hnum;

    util_strlcpy(help, "Help topics: ", help_sz);

    hnum = sizeof(help_commands) / sizeof(help_commands[0]);
    for (ii = 0; ii < hnum; ii++)
    {
        util_strlcat(help, help_commands[ii].help_cmd, help_sz);

        /* Do not add a "," for the last element */
        if ((ii + 1) < hnum)
        {
            util_strlcat(help, ", ", help_sz);
        }
    }
}

/**
 * @}
 */
