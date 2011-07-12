#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

#include "bsd/queue.h"

struct aion_player
{
    LIST_ENTRY(aion_player)     apl_list;       /* Next linked list element      */

    char                        *apl_name;      /* Aion player name         */
    uint32_t                    apl_apvalue;    /* Accumulated AP value     */
};

LIST_HEAD(aion_player_list, aion_player);

/*
 * Currently I'm not sure where to put the structure about US, so lets just keep it separate
 */
struct aion_player      aion_player_self;

struct aion_player_list aion_players_cached;    /* Remembered players   */
struct aion_player_list aion_group;             /* Current group        */

static struct aion_player* aion_player_alloc(char *charname);
static void aion_player_release(struct aion_player *player);
static struct aion_player* aion_group_find(char *charname);
static void aion_group_flush(void);
static void aion_group_dump(void);

bool aion_init(void)
{
    LIST_INIT(&aion_players_cached);
    LIST_INIT(&aion_group);

    aion_player_self.apl_name       = NULL;
    aion_player_self.apl_apvalue    = 0;

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

    /* Scan the list of cached players, if we find it there, return it */
    LIST_FOREACH(curplayer, &aion_players_cached, apl_list)
    {
        if (strcasecmp(curplayer->apl_name, charname) != 0) continue;

        /* Found player, remove it from the cached list and return it */
        LIST_REMOVE(curplayer, apl_list);
        return curplayer;
    }

    curplayer = malloc(sizeof(struct aion_player));
    assert(curplayer != NULL);

    curplayer->apl_name     = strdup(charname);
    curplayer->apl_apvalue  = 0;

    return curplayer;
}

void aion_player_release(struct aion_player *player)
{
    if (player == &aion_player_self)
    {
        assert(!"Unable to remove yourself from the group");
    }

    /* Re-insert it into the cached player list */
    LIST_INSERT_HEAD(&aion_players_cached, player, apl_list);
}

struct aion_player* aion_group_find(char *charname)
{
    struct aion_player *curplayer;

    /* We're always in our group :) */
    if (aion_player_is_self(charname))
    {
        return &aion_player_self;
    }

    LIST_FOREACH(curplayer, &aion_group, apl_list)
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
        if (player == &aion_player_self)
        {
            /* Ok, we joined another group, flush the current one */
            aion_group_flush();
        }

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
    LIST_INSERT_HEAD(&aion_group, player, apl_list);

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
        printf("ERROR: Player %s cannot leave since it's not in the group.\n", charname);
        return true;
    }

    if (player == &aion_player_self)
    {
        /* We left the group, remove all other players from it */
        aion_group_flush();
    }
    else
    {
        LIST_REMOVE(player, apl_list);
        aion_player_release(player);
    }

    aion_group_dump();
    return true;
}

/*
 * Remove all group members from the list
 */
void aion_group_flush(void)
{
    struct aion_player *curplayer;
    struct aion_player *nextplayer;


    LIST_FOREACH_MUTABLE(curplayer, &aion_group, apl_list, nextplayer)
    {
        LIST_REMOVE(curplayer, apl_list);
        aion_player_release(curplayer);
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

void aion_group_dump(void)
{
    struct aion_player *curplayer;

    printf("======= Current group:\n");
    LIST_FOREACH(curplayer, &aion_group, apl_list)
    {
        printf(" * %s: AP = %u\n", curplayer->apl_name, curplayer->apl_apvalue);
    }
    printf("------- Cached \n");
    LIST_FOREACH(curplayer, &aion_players_cached, apl_list)
    {
        printf(" * %s: AP = %u\n", curplayer->apl_name, curplayer->apl_apvalue);
    }

    printf("======\n");
}
