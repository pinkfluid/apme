/*
 * aion.c - APme: Aion Automatic Abyss Point Tracker
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
 * @file aion.c Aion specific functionality, tracking groups, chatlog ...
 * 
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
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
#include "items.h"

/**
 * @defgroup aion Aion Subsystem
 *
 * @brief This is a collection of functions that deal with various aspects of the game.
 * 
 * Before you can use this sub-system, aion_init() must be called.
 *
 * This module is responsible for:
 *      - Tracking of group members
 *      - Processing looted items
 *      - Updating AP statistics
 *      - The language translator
 *      - Probably lots more
 * @{
 */

/**
 * This structures hold information about a player:
 *  - Name
 *  - Accumulated Abyss points
 *  - Chat log (txtbuf)
 *  - Inventory full flag
 *
 * @note A player can be on the cached list and on the
 * group list simultaneously.
 */
struct aion_player
{
    LIST_ENTRY(aion_player)     apl_cached;             /**< Cached linked list element         */
    LIST_ENTRY(aion_player)     apl_group;              /**< Group linked list element          */

    char                        apl_name[AION_NAME_SZ]; /**< Aion player name                   */
    uint32_t                    apl_apvalue;            /**< Accumulated AP value               */
    struct txtbuf               apl_txtbuf;             /**< Text buffer, linked to chat buffer */
    char                        apl_chat[AION_CHAT_SZ]; /**< Chat buffer                        */
    bool                        apl_invfull;            /**< Is inventory full flag             */
};

/** Create the definition of the HEAD structure for the linked list of aion_player structures   */
LIST_HEAD(aion_player_list, aion_player);

/** This is us, the player! */
struct aion_player      aion_player_self;

/** Cached list of players, mainly used for chat history */
struct aion_player_list aion_players_cached;

/**
 * List of players in the current group. This list cannot be empty 
 * so it is assumed that the players itself is always on this list
 *
 * @dot
 *
 *  digraph player_list
 *  {
 *
 *      subgraph cluster0
 *      {
 *          style=filled
 *          color=lightgray
 *
 *          node [fontsize=8.0]
 *          FULL [label="Full Group" shape="none" fontsize="12.0"]
 *
 *          Player -> Member1
 *          Member1 -> Member2
 *          Member2 -> Member3
 *          Member3 -> Member4
 *          Member4 -> Member5
 *          Member5 -> Player
 *
 *          {
 *              rank=sink
 *              FULL
 *          }
 *      }
 *
 *      subgraph cluster1
 *      {
 *          style=filled
 *          color=lightgray
 *
 *          node [fontsize=8.0]
 *
 *          EMPTY [label="Empty Group" shape="none" fontsize="12.0"]
 *          Self [label="Player"]
 *
 *          Self -> Self
 *
 *          {
 *              rank=sink
 *              EMPTY
 *          }
 *      }
 *  }
 *
 * @enddot
 */
struct aion_player_list aion_group;             /* Current group        */

/**
 * If true, exclude the player if it has full invenvtory from the AP fair system.
 * This is confusing for some palyers so it is turned OFF by default.
 */
static bool aion_invfull_exclude = false;

/**
 * Maximum AP a palyer may accumulate before it is free-for-all.
 * 0 means there's no limit.
 *
 * @see aion_aplimit_set
 * @see aion_aplimit_get
 */
static uint32_t aion_ap_limit = 0;

static void aion_player_init(struct aion_player *player, char *name);
static struct aion_player* aion_player_alloc(char *charname);
static struct aion_player* aion_group_find(char *charname);
static void aion_group_dump(void);
static void aion_group_iter_fill(struct aion_group_iter *iter, struct aion_player *player);

/**
 * Initialize the AION sub-system. This must be called before any other functions
 *
 * @retval true on error
 * @retval false on error
 */
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

/**
 * "paste" @p text to the clipboard, this is how we send
 * data to the user via the game chat.
 *
 * @param[in]       text        Text to copy to the clipboard
 *
 * @return
 *      This function just forwards the error from clipboard_set_text()
 */
bool aion_clipboard_set(char *text)
{
    char clip[AION_CLIPBOARD_MAX];

    /* Clip the string */
    util_strlcpy(clip, text, sizeof(clip));

    return clipboard_set_text(clip);
}

/**
 * Initialize a @p aion_player structure with default values
 *
 * @param[in]   charname    Player name
 * @param[out]  player      The aion_player structure
 */ 
void aion_player_init(struct aion_player *player, char *charname)
{
    util_strlcpy(player->apl_name, charname, sizeof(player->apl_name));
    player->apl_apvalue  = 0;
    player->apl_invfull  = false;

    /* Initialize the chat buffers */
    memset(player->apl_chat, 0, AION_CHAT_SZ);
    tb_init(&player->apl_txtbuf, player->apl_chat, AION_CHAT_SZ);
}

/**
 * Test if the given @p charname is ourselves, <I>the player</I>.
 *
 * If charname is NULL, "You" or Player's name, then 
 * we're dealing with ourselves
 *
 * @param[in]       charname        Character name
 *
 * @retval          true            If @p charname is the player
 * @retval          false           If @p charname is not the current palyer
 */ 
bool aion_player_is_self(char *charname)
{
    if (charname == NULL) return true;
    if (strcasecmp(charname, AION_NAME_DEFAULT) == 0) return true;
    if (strcasecmp(aion_player_self.apl_name, charname) == 0) return true;

    return false;
}

/**
 * Set the current player name
 *
 * @param[in]       charname        Player name
 *
 */
void aion_player_name_set(char *charname)
{
    util_strlcpy(aion_player_self.apl_name, charname, sizeof(aion_player_self.apl_name));

    event_signal(EVENT_AION_GROUP_UPDATE);
}

/**
 * Allocate a aion_player structure.
 *
 * If a player with @p charname alraedy exists in the
 * cached list, move the structure to the head of the list
 * and return it.
 *
 * If not, allocate a new structure, register it
 * on the head of the cached list and return it.
 *
 * @param[in]       charname        Player name
 *
 * @return
 *      Always returns a valid @ref aion_player structure
 */
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

/**
 * Cache a chat line from character @p charname
 *
 * @param[in]       charname        Character name
 * @param[in]       chat            Chat line
 *
 * @return
 *      Returns false on error, but that shouldn't happen.
 */
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

/**
 * Retrieve a previously cached chat line for character @p charname
 *
 * @param[in]       charname    Character name
 * @param[in]       msgnum      Chat line number in reverse order (0 = most recent, 1 second most recent)
 * @param[out]      dst         Destination buffer
 * @param[out]      dst_sz      Size of the destinatin buffer
 *
 * @retval          true        On success
 * @retval          false       On error
 */
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

/**
 * Search the current group for a character with the name of @p charname
 *
 * @param[in]       charname    Character name
 *
 * @return 
 *      Returns the associated aion_player structure or NULL if @p charname
 *      was not found in the current group list.
 */
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

/**
 * Add the character with @p charname to the current group list.
 *
 * If the character is already on the list, do nothing.
 *
 * @param[in]       charname    Character name
 * 
 * @retval          true        On success
 * @retval          false       On error
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


/**
 * Remove the character @p charname from the group list.
 *
 * If @p charname is the player itself, disband the group -- we're alone again :(
 *
 * @param[in]       charname        Character name
 *
 * @retval          true            On success
 * @retval          false           On error
 *
 * @bug Always returns true?
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

/**
 * Disband the group.
 *
 * This function removes all characters from the current group list, except 
 * the player itself.
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

/**
 * Process group/alliance item loot
 *
 * If the item looted is an AP relic, update AP statistics for the player
 * @p charname
 *
 * If the player acuqired the item it also means it has room in the inventory,
 * therefore we should clear the inventory full flag.
 *
 * @note
 * This function will also update the application main screen.
 *
 * @param[in]       charname        The player that looted the item
 * @param[in]       itemid          The itemID as parsed from the chatlog
 */
void aion_group_loot(char *charname, uint32_t itemid)
{
    struct aion_player *player;
    struct item *item;

    bool update_stats = false;
   
    item = item_find(itemid);

    /* If the player looted an AP item, add it to the group automatically */
    if ((item != NULL) && (item->item_ap > 0))
    {
        aion_group_join(charname);
    }

    /* If the player is not part of the group, do nothing */
    player = aion_group_find(charname);
    if (player == NULL)
    {
        return;
    }

    /* Check if this player's inventory is marked as full */
    if (player->apl_invfull)
    {
        aion_invfull_set(charname, false);
        update_stats = true;
    }

    if (item != NULL)
    {
        if (item->item_ap > 0)
        {
            aion_group_apvalue_update(charname, item->item_ap);
            update_stats = true;
        }

        con_printf("LOOT: %s -> %s (%u AP)\n", player, item->item_name, item->item_ap);
    }
    else
    {
        con_printf("LOOT: %s -> %u\n", player, itemid);
    }

    /* Update loot statistics */
    if (update_stats)
    {
        char aprolls[CHATLOG_CHAT_SZ];

        aion_group_get_aplootrights(aprolls, sizeof(aprolls));
        aion_clipboard_set(aprolls);
        event_signal(EVENT_AION_LOOT_RIGHTS);
    }
}

/**
 * Update the AP statistics for @p charname
 *
 * @param[in]       charname        Character name
 * @param[in]       apval           Abyss points
 *
 * @retval          true            On success
 * @retval          false           If the character was not found in the current group list
 *
 * @note This function will update the application's main screen.
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

/**
 * Set the accumulated abyss points for @p charname to a specific value
 *
 * @param[in]       charname        Character name
 * @param[in]       apval           Abyss points
 *
 * @retval          true            On success
 * @retval          false           If the character was not found in the current group list
 *
 * @note This function will update the application's main screen.
 */
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

/**
 * Reset the accumulated abyss points for all known characters including the player
 *
 * @note This function will update the application's main screen.
 */
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

/**
 * Scan the current group list and find out the lowest
 * amount of abyss points between the characters
 *
 * @return
 *      The lowest value of Abyss Points
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

/**
 * Set the <I>intentory full</I> flag for @p charname
 * 
 * @param[in]       charname        Character name
 * @param[in]       isfull          Full inventory flag
 *
 * @retval          true            On success
 * @retval          false           If the character was not found in the current group list
 *
 * @note This function will update the application's main screen.
 */
bool aion_invfull_set(char *charname, bool isfull)
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

/**
 * Get the <I>inventory full</I> flag for @p charname
 *
 * @param[in]       charname        Character name
 *
 * @retval          true            If inventory full flag is set
 * @retval          false           If inventory flag not set or error
 *
 * @bug Inventory not full and error both return false and is not possible to
 * differentiate between the two.
 */
bool aion_invfull_get(char *charname)
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

/**
 * Enable or disable enforcment of "inventory full" exclude policy
 *
 * @param[in]       enable      True if clean inventory should be enforced, otherwise false
 *
 * @note The default behavior is "false"
 * @see aion_invfull_excl_get
 */
void aion_invfull_excl_set(bool enable)
{
    aion_invfull_exclude = enable;
}

/**
 * Rerurn the status of the "inventory full" exclude policy:
 *
 * @retval          true        If exclude policy enabled
 * @retval          false       If exclude policy disabled (default)
 *
 * @see aion_invfull_excl_set
 */
bool aion_invfull_excl_get(void)
{
    return aion_invfull_exclude;
}

/**
 * Clear the "inventory full" flag for all players
 * 
 * @note This will update the application's main screen.
 */ 
void aion_invfull_clear(void)
{
    struct aion_player *player;

    LIST_FOREACH(player, &aion_players_cached, apl_cached)
    {
        player->apl_invfull = false;
    }

    /* The current player is not in the global cache list */
    aion_player_self.apl_invfull = false;

    /* Refresh the group list on the main screen */
    event_signal(EVENT_AION_GROUP_UPDATE);
}

/**
 * Set the upper AP limit.
 * 
 * After the AP limit was reached the player will be excluded
 * from the loot rotation. When all players reach the limit,
 * the loot should be FFA.
 * 
 * @param[in]       aplimit     The upper limit of AP (0 means unlimited, default)
 *
 * @note This is a user requested feature, some players just
 * prefer to add some randomness in their loots
 */
void aion_aplimit_set(uint32_t aplimit)
{
    aion_ap_limit = aplimit;

    event_signal(EVENT_AION_GROUP_UPDATE);
}

/**
 * Return the upper AP limit
 * 
 * @return
 *      The AP upper limit value.
 *
 * @see aion_aplimit_set
 */
uint32_t aion_aplimit_get(void)
{
    return aion_ap_limit;
}

/**
 * Get statistics for the current group in text format.
 *
 * @note This is used for ?apstat
 *
 * @param[out]      stats       Statistics character buffer
 * @param[in]       stats_sz    Maximum size of the buffer
 *
 * @retval          true        On success
 *
 * @bug This function never returns false, should it be void?
 */
bool aion_group_get_stats(char *stats, size_t stats_sz)
{
    char curstat[64];
    struct aion_player *player;

    *stats = '\0';

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        snprintf(curstat, sizeof(curstat), "|%s %uAP", player->apl_name, player->apl_apvalue); 
        util_strlcat(stats, curstat, stats_sz);
    }

    return true;
}

/**
 * Get the current AP loot rights in text format.
 *
 * @note This is ued by ?aploot
 *
 * @param[out]      stats       Statistics character buffer
 * @param[in]       stats_sz    Maximum size of the buffer
 *
 * @retval          true        On success
 *
 * @bug This function never returns false, should it be void?
 */
bool aion_group_get_aplootrights(char *stats, size_t stats_sz)
{
    char curstats[64];
    char inv_full_str[256];
    uint32_t lowest_ap;
    struct aion_player *player;
    bool inv_full_stats;
    bool have_stats;

    /* The AP limit value changes the format slightly */
    if (aion_ap_limit > 0)
    {
        snprintf(stats, stats_sz, "Loot rights (<%dAP): ", aion_ap_limit);
    }
    else
    {
        util_strlcpy(stats, "Loot rights: ", stats_sz);
    }

    util_strlcpy(inv_full_str, " | Full inventory: ", sizeof(inv_full_str));

    lowest_ap = aion_group_apvalue_lowest();

    inv_full_stats = false;
    have_stats = false;

    LIST_FOREACH(player, &aion_group, apl_group)
    {
        /* Check if this player has full inventory */
        if (player->apl_invfull)
        {
            /* Display full inventory warning */
            inv_full_stats = true;
            util_strlcat(inv_full_str, player->apl_name, sizeof(inv_full_str));
            util_strlcat(inv_full_str, " ", sizeof(inv_full_str));

            /* If the exclude policy is enabled, this player doesn't get loot :P */
            if (aion_invfull_exclude)
            {
                continue;
            }
        }

        if (player->apl_apvalue > lowest_ap)
        {
            continue;
        }

        if ((aion_ap_limit > 0) &&
            (aion_ap_limit <= player->apl_apvalue))
        {
            con_printf("Player %s has %dAP and is above the limit of %d.\n",
                       player->apl_name,
                       player->apl_apvalue,
                       aion_ap_limit);
            continue;
        }

        util_strlcpy(curstats, player->apl_name, sizeof(curstats));
        util_strlcat(curstats, " ", sizeof(curstats));

        util_strlcat(stats, curstats, stats_sz);
        have_stats = true;
    }

    if (!have_stats)
    {
        /*
         * We didn't produce a single stat since all players seem to be above
         * the AP limit
         */
        util_strlcat(stats, "Free for All", stats_sz);
    }

    con_printf("Stats_sz = %ld\n", (long)stats_sz);
    /* If somebody has inventory full, display that in the stats */
    if (inv_full_stats)
    {
        util_strlcat(stats, inv_full_str, stats_sz);
    }

    return true;
}

/**
 * Initialize a aion_group_iter structure from an aion_player structure
 *
 * This is used for iterating the current group
 *
 * @param[out]      iter        aion_group_iter structure that will be filled with data from @p player
 * @param[in]       player      aion_player structure that will be used to fill the @p iter structure
 *
 * @see aion_group_first
 * @see aion_group_next
 * @see aion_group_end
 */
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

/**
 * Group iterator initializer. This function must be called before
 * any other iterator functions.
 *
 * The iterator functions traverse the group list and fill the
 * itertator with data accordingly. The @p aion_player
 * structure is not exposed outside this module.
 *
 * @param[in,out]       iter        Group iterator
 *
 * Example code:
 * @code 
 *
 * struct aion_group_iter iter;
 *
 * for (aion_group_first(&iter); !aion_group_end(&iter); aion_group_next(&iter))
 * {
 *       printf("%s %d %s\n", iter.agi_name, iter.agi_apvalue, iter.agi_invfull ? "INVFULL" : "");
 * }
 *
 * @endcode
 *
 * @see aion_group_next
 * @see aion_group_end
 */
void aion_group_first(struct aion_group_iter *iter)
{
    struct aion_player *player;

    player = LIST_FIRST(&aion_group);
    aion_group_iter_fill(iter, player);
}

/**
 * Move to the next character in the group
 *
 * @p iter should be initialized with aion_group_first()
 *
 * @param[in,out]       iter        Current/Next player data
 *
 * @see aion_group_first
 * @see aion_group_end
 */
void aion_group_next(struct aion_group_iter *iter)
{
    struct aion_player *player;

    player = LIST_NEXT(iter->__agi_curplayer, apl_group);
    aion_group_iter_fill(iter, player);
}

/**
 * Checks if the current iterator reached end of the list
 *
 * param[in]        iter        Group iterator
 *
 * @retval          true        If end of list was reached
 * @retval          false       If end of list not reached yet
 */
bool aion_group_end(struct aion_group_iter *iter)
{
    if (iter->__agi_curplayer == NULL) return true;

    return false;
}

/**
 *  Debug function that dumps the current group info to the console
 */
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

/**
 * @}
 */
