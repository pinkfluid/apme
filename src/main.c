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
#include <errno.h>

#include "console.h"
#include "aion.h"
#include "cmd.h"
#include "chatlog.h"
#include "util.h"
#include "help.h"
#include "event.h"
#include "version.h"
#include "term.h"
#include "config.h"

/**
 * @defgroup headless Headless Main
 * @brief This is the main module of the terminal (GUI-less) client
 *
 * @{
 */ 

static bool apme_prompt(char *prompt, char *answer);
static void apme_chatlog_check(void);
static void apme_screen_update(void);
static void apme_sys_elevate(void);
static void apme_event_handler(enum event_type ev);
static bool apme_init(int argc, char* argv[]);
static void apme_cfg_apply(void);
static void apme_periodic(void);

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
bool apme_prompt(char *prompt, char *answer)
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
void apme_chatlog_check(void)
{
    bool chatlog_enabled;
    bool enable_ok;

    /* Check if the chatlog feature is enabled in AION */
    if (!aion_chatlog_is_enabled(&chatlog_enabled))
    {
        apme_prompt("Unable to determine if the CHATLOG is enabled. Not all features might be available. Press RETURN to continue.", "");
        return;
    }

    if (chatlog_enabled)
    {
        con_printf("CHATLOG is enabled.\n");
        return;
    }

    /* Do the warn dialog and enable chatlog stuff */
    printf("%s\n", help_chatlog_warning);
    enable_ok = apme_prompt("Press ENTER to enable the chatlog or type NO to abort.",
                            "");
    if (!enable_ok)
    {
        apme_prompt("The CHATLOG was not enabled by user request, press ENTER to continue", "");
        return;
    }

    if (!aion_chatlog_enable())
    {
        printf("%s\n", help_chatlog_enable_error);
        apme_prompt("Press ENTER to continue", "");
        return;
    }

    printf("%s\n", help_chatlog_enabled);
    apme_prompt("Press ENTER to continue", "");
}

/**
 * Updates the applications main terminal screen
 *
 * This is usually called in response to certain events
 */
void apme_screen_update(void)
{
    struct aion_group_iter iter;
    char buf[256];

    /* Reset screen */
    term_clear();

    term_setcolor(TERM_FG_YELLOW);
    printf("***** APme version %s (by Snowsong @ Nexus)\n\n", APME_VERSION_STRING);

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

    if (aion_aploot_rights(buf, sizeof(buf)))
    {
        term_setcolor(TERM_FG_MAGENTA);
        printf("Current AP loot info:\n%s\n", buf);
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
 * Respawn a new instance of the current executable with Admin privileges. If this operation
 * is successfull, the current process will terminate.
 *
 * @note This function will trigger the UAC prompt.
 */
void apme_sys_elevate(void)
{
    apme_prompt("APme was unable to open the system.ovr or Chat.log file due to restricted\npermissions. In order to successfully open these files it needs administrator\nprivileges. Note, that APme will will grant full control of the system.ovr and Chat.log file to Everyone.\n\nPress ENTER to continue", "");

    if (sys_self_elevate())
    {
        /* Just terminate the current process after a successfully elevation */
        exit(0);
    }
    else
    {
        /* Unable to elevate the current process, or user cancelled */
        apme_prompt("Unable to elevate the current process, APme will probably not work correctly. Press ENTER to continue.", "");
    }
}

/**
 * This is the "catch all events" function
 *
 * In the current implementation, this just calls the screen update
 * function
 */
void apme_event_handler(enum event_type ev)
{
    (void)ev;
    
    con_printf("GOT EVENT!\n");
    switch (ev)
    {
        case EVENT_SYS_ELEVATE_REQUEST:
            apme_sys_elevate();
            break;

        default:
            /* Just update the main screen on every other event */
            apme_screen_update();
    }
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
bool apme_init(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    /* First initialize the debug console */
    con_init();

    /* Initialize events early, elevation is requested with events! */
    event_register(apme_event_handler);

    /* Do the chatlog enable/disable stuff, warn user... */
    apme_chatlog_check();

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

    /* Initialize the configuration file */
    if (!cfg_init())
    {
        con_printf("Error initializing the config subsystem.\n");
        /* Non-fatal for now -- we'll revert to defaults */
    }
    else
    {
        /* Apply the loaded configuration */
        apme_cfg_apply();
    }

    return true;
}

/**
 * Check the environment and set the environment name
 */
void apme_cfg_apply(void)
{
    char cfg[1024];

    if (cfg_get_string(CFG_SEC_APP, "name", cfg, sizeof(cfg)))
    {
        con_printf("MAIN: CFG name = %s\n", cfg);
        aion_player_name_set(cfg);
    }

    if (cfg_get_string(CFG_SEC_APP, "apformat", cfg, sizeof(cfg)))
    {
        con_printf("MAIN: CFG apformat = %s\n", cfg);
        aion_aploot_fmt_set(cfg);
    }
}

/**
 * The periodic function, this is called by the main loop periodically
 *
 * It polls the clipboard for commands and the chatlog for new text
 *
 */
void apme_periodic(void)
{
    cmd_poll();
    chatlog_poll();
    cfg_periodic();
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
#include <errno.h>
#include <strings.h>
#include <fcntl.h>

int old(int argc, char *argv[])
{
    /* Initialize APme */
    if (!apme_init(argc, argv))
    {
        return 1;
    }

    /* Show screen */
    apme_screen_update();

    /* Main processing loop */
    for (;;)
    {
        apme_periodic();
        /* Polling rate is 100hz */
        usleep(1000000 / 100);
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

