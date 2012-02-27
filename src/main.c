#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
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
#include "term.h"

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
        aptrack_prompt("Unable to determine if the CHATLOG is enabled. Not all features might be available. Press RETURN to continue.", "");
        return;
    }

    if (chatlog_enabled)
    {
        con_printf("CHATLOG is enabled.\n");
        return;
    }

    /* Do the warn dialog and enable chatlog stuff */
    printf("%s\n", help_chatlog_warning);
    enable_ok = aptrack_prompt("Press ENTER to enable the chatlog or type NO to abort.",
                               "");
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

void aptrack_screen_update(void)
{
    struct aion_group_iter iter;
    char buf[256];

    /* Reset screen */
    term_clear();

    term_setcolor(TERM_FG_YELLOW);
    printf("***** APme version %s (by Playme @ Telemachus)\n\n", APME_VERSION_STRING);

    term_setcolor(TERM_FG_YELLOW);
    term_setcolor(TERM_BG_BLUE);
    printf("=================== Current Group Status ===========");
    term_setcolor(TERM_COLOR_RESET);
    printf("\n\n");

    for (aion_group_first(&iter); !aion_group_end(&iter); aion_group_next(&iter))
    {
        /* Paint ourselves green */
        if (aion_player_is_self(iter.agi_name))
        {
            term_setcolor(TERM_FG_GREEN);
        }
        else
        {
            term_setcolor(TERM_COLOR_RESET);
        }

        printf(" * %-16s (AP: %d) %s\n", iter.agi_name, iter.agi_apvalue, iter.agi_invfull ? " -- FULL INVENTORY" : "");
    }

    printf("\n");
    term_setcolor(TERM_FG_YELLOW);
    term_setcolor(TERM_BG_BLUE);
    printf("====================================================");
    term_setcolor(TERM_COLOR_RESET);
    printf("\n\n");

    if (aion_group_get_aplootrights(buf, sizeof(buf)))
    {
        term_setcolor(TERM_FG_MAGENTA);
        printf("Current AP Relic %s\n", buf);
    }
    term_setcolor(TERM_COLOR_RESET);

    term_setcolor(TERM_FG_CYAN);
    help_usage(buf, sizeof(buf));
    printf("\n%s\n", help_mainscreen);
    printf("%s\n", buf);
    term_setcolor(TERM_COLOR_RESET);

    fflush(stdout);
}

void aptrack_event_handler(enum event_type ev)
{
    (void)ev;
    
    /* Just update the main screen on every event */
    aptrack_screen_update();
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

void aptrack_env(void)
{
    char *default_name = getenv("APME_NAME");

    if (default_name != NULL)
    {
        aion_player_name_set(default_name);
    }
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

    /* Get some stuff from the environment */
    aptrack_env();

    /* Show screen */
    aptrack_screen_update();

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

