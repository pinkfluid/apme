#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>
#include <ctype.h>

#include "bsd/queue.h"

#include "aion.h"
#include "util.h"
#include "txtbuf.h"

#define AION_CHAT_SZ    1024

struct aion_player
{
    LIST_ENTRY(aion_player)     apl_cached;             /* Cached linked list element           */
    LIST_ENTRY(aion_player)     apl_group;              /* Group linked list element            */

    char                        *apl_name;              /* Aion player name                     */
    uint32_t                    apl_apvalue;            /* Accumulated AP value                 */
    struct txtbuf               apl_txtbuf;             /* Text buffer, linked to chat buffer   */
    char                        apl_chat[AION_CHAT_SZ]; /* Chat buffer                          */
};

LIST_HEAD(aion_player_list, aion_player);

/*
 * Currently I'm not sure where to put the structure about US, so lets just keep it separate
 */
struct aion_player      aion_player_self;

struct aion_player_list aion_players_cached;    /* Remembered players   */
struct aion_player_list aion_group;             /* Current group        */

static struct aion_player* aion_player_alloc(char *charname);
static struct aion_player* aion_group_find(char *charname);
static void aion_group_dump(void);

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

    aion_player_self.apl_name       = NULL;
    aion_player_self.apl_apvalue    = 0;

    /* Initialize the chat buffer, just in case */
    memset(aion_player_self.apl_chat, 0, AION_CHAT_SZ);
    tb_init(&aion_player_self.apl_txtbuf, aion_player_self.apl_chat, AION_CHAT_SZ);

    return true;
}

/*
 * If charname is NULL, "You" or Player's name, then 
 * we're dealing with ourselves
 */ 
bool aion_player_is_self(char *charname)
{
    if (charname == NULL) return true;
    if (strcasecmp(charname, "You") == 0) return true;

    if ((aion_player_self.apl_name != NULL) &&
        (strcasecmp(aion_player_self.apl_name, charname) == 0))
    {
        return true;
    }

    return false;
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
        /* Found player -- return it */
        return curplayer;
    }

    curplayer = malloc(sizeof(struct aion_player));
    assert(curplayer != NULL);

    curplayer->apl_name     = strdup(charname);
    curplayer->apl_apvalue  = 0;

    /* Initialize the chat buffers */
    memset(curplayer->apl_chat, 0, AION_CHAT_SZ);
    tb_init(&curplayer->apl_txtbuf, curplayer->apl_chat, AION_CHAT_SZ);

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
        printf("Error caching chat\n");
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

    /* We're always in our group :) */
    if (aion_player_is_self(charname))
    {
        return &aion_player_self;
    }

    LIST_FOREACH(curplayer, &aion_group, apl_group)
    {
        if (strcasecmp(curplayer->apl_name, charname) != 0) continue;

        return curplayer;
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
        aion_group_dump();

        return true;
    }

    player = aion_player_alloc(charname);
    if (player == NULL)
    {
        printf("ERROR: Unable to allocate player\n");
        return false;
    }

    /* Insert this player to the group list */
    LIST_INSERT_HEAD(&aion_group, player, apl_group);

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
        LIST_REMOVE(curplayer, apl_group);
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
        printf("ERROR: Player %s is not in the group.\n", charname);
        return false;
    }

    player->apl_apvalue += apval;

    return true;
}

bool aion_group_apvalue_set(char *charname, uint32_t apval)
{
    struct aion_player *player;

    player = aion_group_find(charname);

    if (player == NULL)
    {
        printf("ERROR: Player %s is not in the group.\n", charname);
        return false;
    }

    player->apl_apvalue = apval;

    return true;
}

bool aion_group_get_stats(char *stats, size_t stats_sz)
{
    char curstat[64];
    struct aion_player *player;

    snprintf(curstat, sizeof(curstat), "| %s (AP %u) ", "You", aion_player_self.apl_apvalue); 
    util_strlcpy(stats, curstat, stats_sz);

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        snprintf(curstat, sizeof(curstat), "| %s (AP %u) ", player->apl_name, player->apl_apvalue); 
        util_strlcat(stats, curstat, stats_sz);
    }

    return true;
}

bool aion_group_get_aprollrights(char *stats, size_t stats_sz)
{
    char curstats[64];
    uint32_t lowest_ap;
    struct aion_player *player;

    lowest_ap = aion_player_self.apl_apvalue;

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        if (player->apl_apvalue < lowest_ap)
        {
            lowest_ap = player->apl_apvalue;
        }
    }

    snprintf(stats, stats_sz, "Roll rights: ");

    /* Do we have the lowest AP? */
    if (aion_player_self.apl_apvalue <= lowest_ap)
    {
        snprintf(curstats, sizeof(curstats), "%s ", "You");
        util_strlcat(stats, curstats, stats_sz);
    }

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        if (player->apl_apvalue > lowest_ap) continue;

        snprintf(curstats, sizeof(curstats), "%s ", player->apl_name);
        util_strlcat(stats, curstats, stats_sz);
    }

    return true;
}



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
            printf("Unknown language\n");
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

void aion_rtranslate(char *txt, uint32_t langid)
{
    char carry = 0;

    while (*txt != '\0')
    {
        int c = *txt;

        if (isalpha(c))
        {
            *txt  = (((c + carry) ^ langid) % 26) + 'a';
            carry = (((c + carry) ^ langid) / 26) + 1;
        }
        else
        {
            carry = 0;
        }

        txt++;
    }
}

void aion_group_dump(void)
{
    struct aion_player *curplayer;

    printf("======= Current group:\n");
    LIST_FOREACH(curplayer, &aion_group, apl_group)
    {
        printf(" * %s: AP = %u\n", curplayer->apl_name, curplayer->apl_apvalue);
    }
    printf("------- Cached \n");
    LIST_FOREACH(curplayer, &aion_players_cached, apl_cached)
    {
        char chat[1024];

        if (!tb_strlast(&curplayer->apl_txtbuf, 0, chat, sizeof(chat)))
        {
            chat[0] = '\0';
        }

        printf(" * %s: AP = %u, lastmsg = %s\n", curplayer->apl_name, curplayer->apl_apvalue, chat);
    }

    printf("======\n");
}
