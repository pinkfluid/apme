#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include <stdio.h>
#include <ctype.h>

#include <pcreposix.h>

#include "bsdqueue/queue.h"

#include "aion.h"
#include "util.h"
#include "txtbuf.h"
#include "console.h"
#include "regeng.h"
#include "event.h"

struct aion_player
{
    LIST_ENTRY(aion_player)     apl_cached;             /* Cached linked list element           */
    LIST_ENTRY(aion_player)     apl_group;              /* Group linked list element            */

    char                        apl_name[AION_NAME_SZ]; /* Aion player name                     */
    uint32_t                    apl_apvalue;            /* Accumulated AP value                 */
    struct txtbuf               apl_txtbuf;             /* Text buffer, linked to chat buffer   */
    char                        apl_chat[AION_CHAT_SZ]; /* Chat buffer                          */
    bool                        apl_invfull;            /* Is inventory full?                   */
};

LIST_HEAD(aion_player_list, aion_player);

/*
 * Currently I'm not sure where to put the structure about US, so lets just keep it separate
 */
struct aion_player      aion_player_self;

struct aion_player_list aion_players_cached;    /* Remembered players   */
struct aion_player_list aion_group;             /* Current group        */

static void aion_player_init(struct aion_player *player, char *name);
static struct aion_player* aion_player_alloc(char *charname);
static struct aion_player* aion_group_find(char *charname);
static void aion_group_dump(void);
static bool aion_get_sysovr_path(char *sysovr_path, size_t sysovr_pathsz);

/*
 * Aion translation tables
 */
struct aion_table
{
    char *at_alpha;
    char *at_next;
};

struct aion_language
{
    struct aion_table al_table[4];
};

/* Asmodian -> elyos language translation table */
struct aion_language aion_lang_asmodian =
{
    .al_table =
    {
        {
            .at_alpha = "ihkjmlonqpsrutwvyxazcbedgf",
            .at_next  = "11111111111111111121222222",
        },
        {
            .at_alpha = "dcfehgjilknmporqtsvuxwzyba",
            .at_next  = "11111111111111111111111122",
        },
        {
            .at_alpha = "edgfihkjmlonqpsrutwvyxazcb",
            .at_next  = "11111111111111111111112122",
        },
        {
            .at_alpha = "@@@@@@@@@@@@@@@@@@@@@@@@@@",
            .at_next  = "33333333333333333333333333",
        }
    }
};

/* Elyos -> Asmodian language translation table */
struct aion_language aion_lang_elyos =
{
    .al_table =
    {
        {
            .at_alpha = "jkhinolmrspqvwtuzGbcJafgde",
            .at_next  = "11111111111111111322322222",
        },
        {
            .at_alpha = "efcdijghmnklqropuvstyzabIJ",
            .at_next  = "11111111111111111111112222",
        },
        {
            .at_alpha = "fgdejkhinolmrspqvwtuzGbcJa",
            .at_next  = "11111111111111111111132222",
        },
        {
            .at_alpha = "ghefklijopmnstqrwxuvGHcdab",
            .at_next  = "11111111111111111111332222",
        }
    }
};

bool aion_init(void)
{
    LIST_INIT(&aion_players_cached);
    LIST_INIT(&aion_group);

    /* Default name */
    aion_player_init(&aion_player_self, AION_NAME_DEFAULT);
    /* Insert the player to the group list, he's not allowed to leave :P */
    LIST_INSERT_HEAD(&aion_group, &aion_player_self, apl_group);

    /* The player should always be in the current group */
    aion_group_join(AION_NAME_DEFAULT);

    return true;
}

void aion_player_init(struct aion_player *player, char *charname)
{
    util_strlcpy(player->apl_name, charname, sizeof(player->apl_name));
    player->apl_apvalue  = 0;
    player->apl_invfull  = false;

    /* Initialize the chat buffers */
    memset(player->apl_chat, 0, AION_CHAT_SZ);
    tb_init(&player->apl_txtbuf, player->apl_chat, AION_CHAT_SZ);
}

/*
 * If charname is NULL, "You" or Player's name, then 
 * we're dealing with ourselves
 */ 
bool aion_player_is_self(char *charname)
{
    if (charname == NULL) return true;
    if (strcasecmp(charname, AION_NAME_DEFAULT) == 0) return true;
    if (strcasecmp(aion_player_self.apl_name, charname) == 0) return true;

    return false;
}

void aion_player_name_set(char *charname)
{
    util_strlcpy(aion_player_self.apl_name, charname, sizeof(aion_player_self.apl_name));
}

struct aion_player* aion_player_alloc(char *charname)
{
    struct aion_player *curplayer;

    if (aion_player_is_self(charname))
    {
        return &aion_player_self;
    }

    /* Scan the list of cached players, if we find it there, return it */
    LIST_FOREACH(curplayer, &aion_players_cached, apl_cached)
    {
        if (strcasecmp(curplayer->apl_name, charname) != 0) continue;

        /* Found player -- move it to the head of the list and return it */
        LIST_REMOVE(curplayer, apl_cached);
        LIST_INSERT_HEAD(&aion_players_cached, curplayer, apl_cached);

        return curplayer;
    }

    curplayer = malloc(sizeof(struct aion_player));
    assert(curplayer != NULL);

    aion_player_init(curplayer, charname);

    /* Add the player to the cached list */
    LIST_INSERT_HEAD(&aion_players_cached, curplayer, apl_cached);

    return curplayer;
}

/* Cache the chat, so we can use all sorts of useful tricks */
bool aion_player_chat_cache(char *charname, char *chat)
{
    struct aion_player *player;

    player = aion_player_alloc(charname);
    if (player == NULL)
    {
        con_printf("Error caching chat\n");
        return false;
    }

    tb_strput(&player->apl_txtbuf, chat);

    return true;
}

bool aion_player_chat_get(char *charname, int msgnum, char *dst, size_t dst_sz)
{
    struct aion_player *player;

    player = aion_player_alloc(charname);
    if (player == NULL)
    {
        return false;
    }

    return tb_strlast(&player->apl_txtbuf, msgnum, dst, dst_sz);
}

struct aion_player* aion_group_find(char *charname)
{
    struct aion_player *curplayer;

    if (aion_player_is_self(charname))
    {
        return &aion_player_self;
    }

    LIST_FOREACH(curplayer, &aion_group, apl_group)
    {
        if (strcasecmp(curplayer->apl_name, charname) == 0)
        {
            return curplayer;
        }
    }

    return NULL;
}

/*
 * Add a character to the current group list
 */
bool aion_group_join(char *charname)
{
    struct aion_player *player;
   
    /* Check if the player is already in the group */
    player = aion_group_find(charname);
    if (player != NULL)
    {
        event_signal(EVENT_AION_GROUP_UPDATE);
        aion_group_dump();
        return true;
    }

    player = aion_player_alloc(charname);
    if (player == NULL)
    {
        con_printf("ERROR: Unable to allocate player\n");
        return false;
    }

    /* Reset stats */
    player->apl_invfull = false;

    /* Insert this player to the group list */
    LIST_INSERT_HEAD(&aion_group, player, apl_group);

    event_signal(EVENT_AION_GROUP_UPDATE);
    aion_group_dump();

    return true;
}

/*
 * Remove a character from the current group list
 */ 
bool aion_group_leave(char *charname)
{
    struct aion_player *player;

    player = aion_group_find(charname);
    if (player == NULL)
    {
        /* Player was not in the group list, so no need to remove it :( */
        return true;
    }

    if (player == &aion_player_self)
    {
        /* We left the group, remove all other players from it */
        aion_group_disband();
    }
    else
    {
        LIST_REMOVE(player, apl_group);
    }

    event_signal(EVENT_AION_GROUP_UPDATE);
    aion_group_dump();

    return true;
}

/*
 * Remove all group members from the list
 */
void aion_group_disband(void)
{
    struct aion_player *curplayer;
    struct aion_player *nextplayer;

    LIST_FOREACH_MUTABLE(curplayer, &aion_group, apl_group, nextplayer)
    {
        /* Never remove us from the list */
        if (curplayer != &aion_player_self)
        {
            LIST_REMOVE(curplayer, apl_group);
        }
    }
}

/*
 * Update AP value of the player's name
 */
bool aion_group_apvalue_update(char *charname, uint32_t apval)
{
    struct aion_player *player;

    player = aion_group_find(charname);
    if (player == NULL)
    {
        con_printf("ERROR: Player %s is not in the group.\n", charname);
        return false;
    }

    player->apl_apvalue += apval;

    event_signal(EVENT_AION_AP_UPDATE);

    return true;
}

bool aion_group_apvalue_set(char *charname, uint32_t apval)
{
    struct aion_player *player;

    player = aion_group_find(charname);

    if (player == NULL)
    {
        con_printf("ERROR: Player %s is not in the group.\n", charname);
        return false;
    }

    player->apl_apvalue = apval;

    event_signal(EVENT_AION_AP_UPDATE);

    return true;
}

/*
 * Find what's the lowest AP in the group
 */
uint32_t aion_group_apvalue_lowest(void)
{
    struct aion_player *player;
    uint32_t lowest_ap;

    lowest_ap = aion_player_self.apl_apvalue;

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        /* Skip players that have full inventory :P */
        if (player->apl_invfull) continue;

        if (player->apl_apvalue < lowest_ap)
        {
            lowest_ap = player->apl_apvalue;
        }
    }

    return lowest_ap;
}

bool aion_group_invfull_set(char *charname, bool isfull)
{
    struct aion_player *player;

    player = aion_group_find(charname);
    if (player == NULL)
    {
        con_printf("invfull_set(): Unable to find player: %s\n", charname);
        return false;
    }

    player->apl_invfull = isfull;

    event_signal(EVENT_AION_INVENTORY_FULL);

    return true;
}

bool aion_group_invfull_get(char *charname)
{
    struct aion_player *player;

    player = aion_group_find(charname);
    if (player == NULL)
    {
        con_printf("invfull_get(): Unable to find player: %s\n", charname);
        return false;
    }

    return player->apl_invfull;
}

bool aion_group_get_stats(char *stats, size_t stats_sz)
{
    char curstat[64];
    struct aion_player *player;

    *stats = '\0';

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        snprintf(curstat, sizeof(curstat), "| %s (AP %u) ", player->apl_name, player->apl_apvalue); 
        util_strlcat(stats, curstat, stats_sz);
    }

    return true;
}

bool aion_group_get_aplootrights(char *stats, size_t stats_sz)
{
    char curstats[64];
    char inv_full_str[256];
    uint32_t lowest_ap;
    struct aion_player *player;
    bool inv_full_stats;

    util_strlcpy(stats, "Loot rights: ", stats_sz);
    util_strlcpy(inv_full_str, " | Full inventory: ", sizeof(inv_full_str));

    lowest_ap = aion_group_apvalue_lowest();

    inv_full_stats = false;

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        if (player->apl_invfull)
        {
            /* Players with full inventory do not get to loot :P */
            inv_full_stats = true;
            util_strlcat(inv_full_str, player->apl_name, sizeof(inv_full_str));
            util_strlcat(inv_full_str, " ", sizeof(inv_full_str));
            continue;
        }
        else if (player->apl_apvalue > lowest_ap)
        {
            continue;
        }

        util_strlcpy(curstats, player->apl_name, sizeof(curstats));
        util_strlcat(curstats, " ", sizeof(curstats));

        util_strlcat(stats, curstats, stats_sz);
    }

    /* If somebody has inventory full, display that in the stats */
    if (inv_full_stats)
    {
        util_strlcat(stats, inv_full_str, stats_sz);
    }

    event_signal(EVENT_AION_LOOT_RIGHTS);

    return true;
}

void aion_group_dump(void)
{
    struct aion_player *curplayer;

    con_printf("======= Current group:\n");
    LIST_FOREACH(curplayer, &aion_group, apl_group)
    {
        con_printf(" * %s: AP = %u\n", curplayer->apl_name, curplayer->apl_apvalue);
    }

    con_printf("------- Cached \n");
    LIST_FOREACH(curplayer, &aion_players_cached, apl_cached)
    {
        char chat[AION_CHAT_SZ];

        if (!tb_strlast(&curplayer->apl_txtbuf, 0, chat, sizeof(chat)))
        {
            chat[0] = '\0';
        }

        con_printf(" * %s: AP = %u, lastmsg = %s\n", curplayer->apl_name, curplayer->apl_apvalue, chat);
    }

    con_printf("======\n");
}

#ifndef USE_NEW_TRANSLATE
void aion_translate(char *txt, uint32_t langid)
{
    int index;
    struct aion_language *lang = NULL;

    switch (langid)
    {
        case LANG_ASMODIAN:
            lang = &aion_lang_asmodian;
            break;

        case LANG_ELYOS:
            lang = &aion_lang_elyos;
            break;

        default:
            con_printf("Unknown language\n");
            return;
    }

    index = 0;

    while (*txt != '\0')
    {
        int c = tolower((int)*txt);

        if (isalpha(c))
        {
            int offset = c - 'a';
            *txt  = lang->al_table[index].at_alpha[offset];
            index = lang->al_table[index].at_next[offset] - '0';
        }
        else
        {
            index = 0;
        }

        txt++;
    }
}

#else /* USE_NEW_TRANSLATE */

/* This translator produces lots of capital text, so it's not that good */
void aion_translate(char *txt, uint32_t langid)
{
    int carry = 0;

    while (*txt != '\0')
    {
        int input = tolower(*txt);
        if (isalpha(input))
        {
            int output;
            int base = 'a';

            input = input - base;
            while (input < 128)
            {
                output = (input ^ langid);
                output -= carry;

                if (isalpha(output))
                {
                    *txt = output;
                    carry = (((output + carry) ^ langid) / 26) + 1;
                    break;
                }

                input+=26;
            }
        }
        else
        {
            carry = 0;
        }

        txt++;
    }

    return;
}
#endif /* USE_NEW_TRNASLATE */

void aion_rtranslate(char *txt, uint32_t langid)
{
    char carry = 0;

    while (*txt != '\0')
    {
        int input = *txt;

        if (isalpha(input))
        {
            int output;

            output = (((input + carry) ^ langid) % 26) + 'a';
            carry = (((input + carry) ^ langid) / 26) + 1;

            *txt = output;
        }
        else
        {
            carry = 0;
        }

        txt++;
    }
}

/*
 * Retrieve the default Aion isntall path from the registry
 */ 
char *aion_install_reg_keys[] =
{
    "SOFTWARE\\NCsoft\\AionEU",     /* Aion Europe  */
    "SOFTWARE\\NCsoft\\Aion",       /* Aion US ?    */
};

char* aion_default_install_path(void)
{
#ifdef SYS_WINDOWS
    static char default_install_path[1024];
    size_t ii;
    bool retval;

    for (ii = 0; ii < sizeof(aion_install_reg_keys) / sizeof(aion_install_reg_keys[0]); ii++)
    {
        retval = reg_read_key(aion_install_reg_keys[ii],
                              "InstallPath",
                              default_install_path,
                              sizeof(default_install_path));
        if (!retval)
        {
            /* Try next key on error */
            continue;
        }

        return default_install_path;
    }

    return NULL;
#else
    /* On Linux, just return the current directory. Used for debugging mainly. */
    return "./";
#endif
}

#define RE_SYSTEM_OVR "^ *([a-zA-Z0-9_]+) *= *\"?([0-9]+)\"?"

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
    util_strlcat(sysovr_path, "\\", sysovr_pathsz);
    util_strlcat(sysovr_path, AION_SYSOVR_FILE, sysovr_pathsz);

    return true;
}

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

    return true;
}

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
    fprintf(sysovr_file, "-- Added by APTRACKER, remove the line below to disable chat logging\n");
    fprintf(sysovr_file, "%s=\"1\"\n", AION_SYSOVR_CHATLOG);

    fclose(sysovr_file);

    return true;
}
