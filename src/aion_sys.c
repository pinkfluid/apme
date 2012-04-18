/*
 * aion_sys.c - APme: Aion Automatic Abyss Point Tracker
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
 * Aion System Functions
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 *
 */

/**
 * @defgroup aion_sys Aion System Functions
 *
 * @brief Various Aion system functions for dealing with
 * registry keys, installation path ...
 *
 * @{
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

#include <pcreposix.h>

#include "util.h"
#include "console.h"
#include "regeng.h"
#include "aion.h"

static bool aion_get_sysovr_path(char *sysovr_path, size_t sysovr_pathsz);

/**
 * Aion registry keys structure
 */ 
struct aion_reg_keys
{
    char *ark_path;         /**< Path to the registry key       */
    char *ark_key;          /**< Key name                       */
};

/**
 * Aion install path registry keys
 *
 * These point to various registry paths used by
 * the Aion game client. APme uses hese ones to
 * retrieve the installation folder where the
 * chat log is located.
 */
struct aion_reg_keys aion_install_reg_keys[] =
{
    {
        /* Pay2Win Aion from GameForge */
        "SOFTWARE\\AION Free-To-Play",
        "Path",
    },
    {
        /* Aion Europe  */
        "SOFTWARE\\NCsoft\\AionEU",
        "InstallPath",
    },
    {   
        /* Aion US ?    */
        "SOFTWARE\\NCsoft\\Aion",
        "InstallPath",
    },
};

/**
 * Retrieve the Aion default installation path
 *
 * If APME_AION_PATH is set, it is used as the default installation path
 * for Aion. Otherwise the registry is scanned for known registry keys
 * used by the game client.
 *
 * @return
 *      Installation path or NULL on error.
 */
char *aion_default_install_path(void)
{
#ifdef SYS_WINDOWS
    static char default_install_path[1024] = "";
    size_t ii;
    bool retval;

    char *envpath;

    if (default_install_path[0] != '\0')
    {
        return default_install_path;
    }

    /* Check the APME_AION_PATH varible, if not set, try to figure out stuff from the registry */
    envpath = getenv("APME_AION_PATH");
    if (envpath != NULL)
    {
        util_strlcpy(default_install_path, envpath, sizeof(default_install_path));
        return default_install_path;
    }

    for (ii = 0; ii < sizeof(aion_install_reg_keys) / sizeof(aion_install_reg_keys[0]); ii++)
    {
        con_printf("INSTALL: Trying %s/%s...\n", aion_install_reg_keys[ii].ark_path,
                                                 aion_install_reg_keys[ii].ark_key);

        retval = reg_read_key(aion_install_reg_keys[ii].ark_path,
                              aion_install_reg_keys[ii].ark_key,
                              default_install_path,
                              sizeof(default_install_path));
        if (!retval)
        {
            /* Try next key on error */
            continue;
        }

        con_printf("INSTALL: Found: %s\n", default_install_path);

        return default_install_path;
    }

    con_printf("INSTALL: Not found\n");
    return NULL;
#else
    /* On Linux, just return the current directory. Used for debugging mainly. */
    return "./";
#endif
}

/** Pattern for matching system.ovr values */
#define RE_SYSTEM_OVR "^ *([a-zA-Z0-9_]+) *= *\"?([0-9]+)\"?"

/**
 * Figure out the full path of the system.ovr file
 *
 * @param[out]      sysovr_path     Character buffer pointing to the full
 *                                  system.ovr path
 * @param[in]       sysovr_pathsz   Size of the buffer pointed to by sysovr_path
 *
 * @retval          true            On success
 * @retval          false           On error
 */
bool aion_get_sysovr_path(char *sysovr_path, size_t sysovr_pathsz)
{
    char *aion_install;

    aion_install = aion_default_install_path();
    if (aion_install == NULL)
    {
        con_printf("Error retrieving Aion install path.\n");
        return false;
    }

    util_strlcpy(sysovr_path, aion_install, sysovr_pathsz);
#ifdef SYS_WINDOWS
    util_strlcat(sysovr_path, "\\", sysovr_pathsz);
#else
    util_strlcat(sysovr_path, "/", sysovr_pathsz);
#endif

    util_strlcat(sysovr_path, AION_SYSOVR_FILE, sysovr_pathsz);

    return true;
}

/**
 * Check if the chatlog feature of the Aion game client is enabled.
 *
 * @param[out]      isenabled       True if the chatlog is enabled, false
 *                                  otherwise
 * @retval          true            On success (this doesn't mean that the
 *                                  chatlog is enabled)
 * @retval          false           Error, unable to verify if the chatlog
 *                                  is enabled.
 */
bool aion_chatlog_is_enabled(bool *isenabled)
{
    char sysovr_path[1024];
    char sysovr_line[1024];
    regex_t re_sysovr;
    FILE *sysovr_file;
    int retval;

    retval = regcomp(&re_sysovr, RE_SYSTEM_OVR, REG_EXTENDED);
    if (retval != 0)
    {
        con_printf("Error compiling system.ovr regex: %s\n", RE_SYSTEM_OVR);
        return false;
    }

    if (!aion_get_sysovr_path(sysovr_path, sizeof(sysovr_path)))
    {
        con_printf("Unable to retrieve the full path to SYSTEM.OVR\n");
        return false;
    }

    con_printf("SYSTEM.OVR full path is '%s'\n", sysovr_path);

    sysovr_file = fopen(sysovr_path, "r");
    if (sysovr_file == NULL)
    {
        /* The system.ovr file does nto exist, chatlog not enabled */
        con_printf("Unable to open system.ovr file\n");
        *isenabled = false;
        return true;
    }

    *isenabled = false;
    /* Parse the system.ovr file */
    while (fgets(sysovr_line, sizeof(sysovr_line), sysovr_file) != NULL)
    {
        regmatch_t rem[3];
        char sysovr_cmd[64];
        char sysovr_val[64];

        /* Match config line */
        retval = regexec(&re_sysovr, sysovr_line, sizeof(rem) / sizeof(rem[0]), rem, 0);
        if (retval != 0)
        {
            /* No match */
            continue;
        }

        re_strlcpy(sysovr_cmd, sysovr_line, sizeof(sysovr_cmd), rem[1]);
        re_strlcpy(sysovr_val, sysovr_line, sizeof(sysovr_val), rem[2]);

        con_printf("Sysovr match: %s = %s\n", sysovr_cmd, sysovr_val);

        if (strcasecmp(sysovr_cmd, AION_SYSOVR_CHATLOG) == 0)
        {
            /* Check if chatlog is enabled */
            if (atoi(sysovr_val) > 0)
            {
                *isenabled = true;
            }
            break;
        }
    }

    fclose(sysovr_file);

    return true;
}

/**
 * Enable the chatlog feature of the Aion game client
 *
 * This function enables the chatlog feature of the Aion
 * game client by writing "g_chatlog=1" in the system.ovr
 * file. If system.ovr does not exist it is created,
 * otherwise the string is appended at the end of the file.
 *
 * @retval      true        If chatlog was successfully enabled
 * @retval      false       If an error occurred
 */
bool aion_chatlog_enable(void)
{
    char sysovr_path[1024];
    FILE *sysovr_file;

    if (!aion_get_sysovr_path(sysovr_path, sizeof(sysovr_path)))
    {
        con_printf("Unable to retrieve the full path to SYSTEM.OVR\n");
        return false;
    }

    /* Open the file in APPEND mode */
    sysovr_file = fopen(sysovr_path, "a");
    if (sysovr_file == NULL)
    {
        con_printf("Unable to open system.ovr file\n");
        return false;
    }

    /* Write out the config */
    fprintf(sysovr_file, "\n");
    fprintf(sysovr_file, "-- Added by APme, remove the line below to disable chat logging\n");
    fprintf(sysovr_file, "%s=\"1\"\n", AION_SYSOVR_CHATLOG);

    fclose(sysovr_file);

    return true;
}

/**
 * @}
 */
