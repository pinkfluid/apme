#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
static void aion_group_iter_fill(struct aion_group_iter *iter, struct aion_player *player);

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

    event_signal(EVENT_AION_GROUP_UPDATE);
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
        /* No need to send an event here, aion_group_disband() does that for us */
    }
    else
    {
        LIST_REMOVE(player, apl_group);
        /* Update with the new status */
        event_signal(EVENT_AION_GROUP_UPDATE);
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
        /* Never remove us from the list */
        if (curplayer != &aion_player_self)
        {
            LIST_REMOVE(curplayer, apl_group);
        }
    }

    event_signal(EVENT_AION_GROUP_UPDATE);
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
    player->apl_invfull = false;

    event_signal(EVENT_AION_AP_UPDATE);

    return true;
}

void aion_apvalue_reset(void)
{
    struct aion_player *player;

    /* Reset statistics for ALL players */
    LIST_FOREACH(player, &aion_players_cached, apl_cached)
    {
        player->apl_apvalue = 0;
        player->apl_invfull = false;
    }

    /* The current player is not in the global cache list */
    aion_player_self.apl_apvalue = 0;
    aion_player_self.apl_invfull = false;

    event_signal(EVENT_AION_AP_UPDATE);
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

    return true;
}

void aion_group_iter_fill(struct aion_group_iter *iter, struct aion_player *player)
{
    iter->__agi_curplayer  = player;
    if (player == NULL) 
    {
        iter->agi_name      = "";
        iter->agi_apvalue   = 0;
        iter->agi_invfull   = false;
    }
    else
    {
        iter->agi_name      = player->apl_name;
        iter->agi_apvalue   = player->apl_apvalue;
        iter->agi_invfull   = player->apl_invfull;
    }
}

void aion_group_first(struct aion_group_iter *iter)
{
    struct aion_player *player;

    player = LIST_FIRST(&aion_group);
    aion_group_iter_fill(iter, player);
}

void aion_group_next(struct aion_group_iter *iter)
{
    struct aion_player *player;

    player = LIST_NEXT(iter->__agi_curplayer, apl_group);
    aion_group_iter_fill(iter, player);
}

bool aion_group_end(struct aion_group_iter *iter)
{
    if (iter->__agi_curplayer == NULL) return true;

    return false;
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

