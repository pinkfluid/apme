#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "console.h"
#include "aion.h"
#include "cmd.h"
#include "chatlog.h"
#include "util.h"
#include "help.h"
#include "event.h"
#include "version.h"

bool aptrack_prompt(char *prompt, char *answer)
{
    char line[64];

    fprintf(stdout, "\n%s > ", prompt); fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        line[0] = '\0';
    }

    fprintf(stdout, "\n"); fflush(stdout);

    util_chomp(line);

    if (strcasecmp(answer, line) == 0) 
    {
        return true;
    }

    return false;
}

void aptrack_chatlog_check(void)
{
    bool chatlog_enabled;
    bool enable_ok;

    /* Check if the chatlog feature is enabled in AION */
    if (!aion_chatlog_is_enabled(&chatlog_enabled))
    {
        aptrack_prompt("Unable to determine if the CHATLOG is enabled. Not all features might be available. Press RETURN to contunue.", "");
        return;
    }

    if (chatlog_enabled)
    {
        con_printf("CHATLOG is enabled.\n");
        return;
    }

    /* Do the warn dialog and enable chatlog stuff */
    printf("%s\n", help_chatlog_warning);
    enable_ok = aptrack_prompt("Type ACCEPT to enable the chatlog, or ENTER to continue",
                               "accept");
    if (!enable_ok)
    {
        aptrack_prompt("The CHATLOG was not enabled by user request, press ENTER to continue", "");
        return;
    }

    if (!aion_chatlog_enable())
    {
        printf("%s\n", help_chatlog_enable_error);
        aptrack_prompt("Press ENTER to continue", "");
        return;
    }

    printf("%s\n", help_chatlog_enabled);
    aptrack_prompt("Press ENTER to continue", "");
}

void aptrack_group_show(void)
{
    struct aion_group_iter iter;
    char buf[256];

    /* Reset screen */
    printf("\x1b" "c");

    printf("\x1b[33m");
    printf("***** APme version %s (by Playme @ Telemachus)\n\n", APME_VERSION_STRING);
    printf("\x1b[33;44m");
    printf("=================== Current Group Status ===========\n\n");
    printf("\x1b[m");
    for (aion_group_first(&iter); !aion_group_end(&iter); aion_group_next(&iter))
    {
        /* Paint ourselves green */
        if (aion_player_is_self(iter.agi_name))
        {
            printf("\x1b[32m");
        }
        else
        {
            printf("\x1b[m");
        }

        printf(" * %-16s (AP: %d) %s\n", iter.agi_name, iter.agi_apvalue, iter.agi_invfull ? " -- FULL INVENTORY" : "");
    }
    printf("\x1b[33;44m");
    printf("\n====================================================\n");
    printf("\x1b[m");
    printf("\n");

    (void)buf;
    if (aion_group_get_aplootrights(buf, sizeof(buf)))
    {
        printf("\x1b[35m");
        printf("Current AP Relic %s\n", buf);
    }
    printf("\x1b[m");
    fflush(stdout);
}

void aptrack_event_handler(enum event_type ev)
{
    switch (ev)
    {
        case EVENT_AION_GROUP_UPDATE:
            aptrack_group_show();
            break;

        case EVENT_AION_AP_UPDATE:
            aptrack_group_show();
            break;

        case EVENT_AION_INVENTORY_FULL:
            aptrack_group_show();
            break;

        case EVENT_AION_LOOT_RIGHTS:
            aptrack_group_show();
            break;
    }
}

bool aptrack_init(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    con_init();

    if (!aion_init())
    {
        con_printf("Unable to initialize the Aion subsystem.\n");
        return false; 
    }

    if (!chatlog_init())
    {
        con_printf("Error initializing the Chatlog parser.\n");
        return false;
    }

    event_register(aptrack_event_handler);

    return true;
}

void aptrack_periodic()
{
    cmd_poll();
    chatlog_poll();
}

int main(int argc, char *argv[])
{
    /* Do the chatlog enable/disable stuff, warn user... */
    aptrack_chatlog_check();

    /* Initialize sub-systems */
    if (!aptrack_init(argc, argv))
    {
        return 1;
    }

    /* Show screen */
    aptrack_group_show();

    /* Main processing loop */
    for (;;)
    {
        aptrack_periodic();
        usleep(300);
    }

#ifdef SYS_WINDOWS
    /* On windows, pause before exiting */
    {
        char buf[16];
        printf("Press ENTER to exit.\n");
        fgets(buf, sizeof(buf), stdin);
    }
#endif

    /* And we disappear into the void */
    return 0;
}

