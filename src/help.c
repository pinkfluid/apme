#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "help.h"
#include "util.h"
#include "console.h"

struct help_entry
{
    char *help_cmd;
    char *help_usage;
    char *help_text;
};

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

const char *help_chatlog_enabled =
"The chatlog feature was enabled in AION.\n"
"You must restart the game client to apply the new settings.\n"
"\n"
"At any point in time, you can remove the SYSTEM.OVR file in\n"
"the client directory to disable this feature.\n"
"\n";

const char *help_chatlog_enable_error =
"Error enabling the CHATLOG feature. Restart the application to try again.\n";

const char *help_mainscreen = 
"Type \"?command <PARAMS>\", selecte the text and copy-paste.\nFor more info, please use the \"?help <TOPIC>\" command.\n";

const char *help_invfull_on =
"ON: Users with full inventory will be temporarily excluded from the AP fair loot system.";

const char *help_invfull_off = 
"OFF: Users with full inventory will be warned only.";

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
        "Set the upper limit of AP per player. If the whole group is above the AP limit, loot is free for all. For example, to enable what commonly referred as the RR2400 system, use ?aplimit 2400"
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
