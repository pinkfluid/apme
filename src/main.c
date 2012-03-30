/*
 * main.c - APme: Aion Automatic Abyss Point Tracker
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
 * Main Terminal Application 
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
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

/**
 * @defgroup headless Headless Main
 * @brief This is the main module of the terminal (GUI-less) client
 *
 * @{
 */ 

/**
 * Simple "prompt a question and wait for an answer" function
 *
 * This is mainly used during startup to ask various questions
 *
 * @param[in]       prompt      Text to prompt
 * @param[in]       answer      Expected answer
 *
 * @return
 * If the answer is equals to the @p answer parameter
 * return true, otherwise false
 */
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

/**
 * Check if the chatlog is enabled
 *
 * If the chatlog is not enabled, ask the user for permission to enable it
 */
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

/**
 * Updates the applications main terminal screen
 *
 * This is usually called in response to certain events
 */
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

/**
 * This is the "catch all events" function
 *
 * In the current implementation, this just calls the screen update
 * function
 */
void aptrack_event_handler(enum event_type ev)
{
    (void)ev;
    
    /* Just update the main screen on every event */
    aptrack_screen_update();
}

/**
 * Initialize the application:
 *      - Initialize the debug console
 *      - Initialize the Aion subsystem
 *      - Initialize the chatlog engine
 *      - Register events
 *
 * @param[in]   argc        Argument number (passed from main) -- not used
 * @param[in]   argv        Argument array (passed from main) -- not used
 *
 * @retval      true        On success
 * @retval      false       If it fails to initialize the aion sub-system
 * @retval      false       If it fails to initialize the chatlog engine
 */
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

/**
 * Check the environment and set the environment name
 */
void aptrack_env(void)
{
    char *default_name = getenv("APME_NAME");

    if (default_name != NULL)
    {
        aion_player_name_set(default_name);
    }
}

/**
 * The periodic function, this is called by the main loop periodically
 *
 * It polls the clipboard for commands and the chatlog for new text
 *
 */
void aptrack_periodic(void)
{
    cmd_poll();
    chatlog_poll();
}

/**
 * The terminal application main entry function
 *
 * @param[in]   argc        Argument number (passed from main) -- not used
 * @param[in]   argv        Argument array (passed from main) -- not used
 *
 * @return
 * 0 on success, any other number on error.
 */
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


/**
 * @}
 */ 
