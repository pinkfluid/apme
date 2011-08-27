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
"This application can enable the chatlog feature for you, but keep in\n"
"mind that  using it was never officially aproved by NCSoft.\n"
"\n"
"Said that, you most probably WONT get banned, but please be aware\n"
"that you are walking on the gray area until there is an official answer\n"
"by NCSoft.\n"
"\n"
"This application is no more illegal than any DPS meter is, since the\n"
"principles behind are the same.\n"
"\n"
"!!!!! WARNING!!!!!! !!!!! WARNING!!!!!! !!!!! WARNING!!!!!! \n";

const char *help_chatlog_enabled =
"The chatlog feature was enabled in AION.\n"
"You must restart the game client to apply the new settings.\n"
"\n"
"At any point in time, you can remove the SYSTEM.OVR file in\n"
"the client directory to disable this feature.\n"
"\n";

const char *help_chatlog_enable_error =
"Error enabling the CHATLOG feature. Restart the application to try again.\n";


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
        "apset",
        "?apset <PLAYER> <AP>",
        "Set the AP points of <PLAYER> to <AP>."
    },
    {
        "gradd",
        "?gradd <PLAYER>",
        "Add <PLAYER> to your group, in case it is not autodetected."
    },
    {
        "grdel",
        "?grdel <PLAYER>",
        "Remove <PLAYER> from your group, in case it is not autodetected. ?grdel You -- disbands the group."
    },
    {
        "elyos",
        "elyos <TEXT>",
        "Translate <TEXT> to elyos/from asmodian."
    },
    {
        "asmo",
        "asmo <TEXT>",
        "Translate <TEXT> to asmodian/from elyos.",
    },
    {
        "relyos",
        "relyos <TEXT>",
        "Reverse of the ?elyos command.",
    },
    {
        "reylos",
        "rasmo <TEXT>",
        "Revers of the ?asmo command.",
    },
    {
        "apcalc",
        "apcalc <RELIC ID> or <RELIC_ID>xN",
        "Calculate value of relic, For example ?apcalc <Major Ancient Crown>x5"
    },
    {
        "echo",
        "echo <TEXT>",
        "Echoes <TEXT> back. Useful for inspecting chat history. For example ?echo !<PLAYER> or ?echo !<PLAYER>-1"
    },
    {
        "chathist",
        "chathist -- ?command !<PLAYER_NAME> or ?command !<PLAYER_NAME>-N",
        "Execute ?command but replace '!<PLAYER_NAME>' with the last text <PLAYER_NAME> entered into chat. Add -1 for second last text..."
    }
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
