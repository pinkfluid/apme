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

struct aion_player_list aion_players_cached;    /* Remembered players   */
struct aion_player_list aion_group;             /* Current group        */

static struct aion_player* aion_player_alloc(char *charname);
static void aion_player_release(struct aion_player *player);
static struct aion_player* aion_group_find(char *charname);
static void aion_group_dump(void);

bool aion_init(void)
{
    LIST_INIT(&aion_players_cached);
    LIST_INIT(&aion_group);

    return true;
}

struct aion_player* aion_player_alloc(char *charname)
{
    struct aion_player *curplayer;

    /* Scan the list of cached players, if we find it there, return it */
    LIST_FOREACH(curplayer, &aion_players_cached, apl_list)
    {
        if (strcasecmp(curplayer->apl_name, charname) == 0)
        {
            return curplayer;
        }
    }

    curplayer = malloc(sizeof(struct aion_player));
    assert(curplayer != NULL);

    curplayer->apl_name     = strdup(charname);
    curplayer->apl_apvalue  = 0;

    return curplayer;
}

void aion_player_release(struct aion_player *player)
{
    /* Re-insert it into the cached player list */
    LIST_INSERT_HEAD(&aion_players_cached, player, apl_list);
}

struct aion_player* aion_group_find(char *charname)
{
    struct aion_player *curplayer;

    LIST_FOREACH(curplayer, &aion_group, apl_list)
    {
        if (strcasecmp(curplayer->apl_name, charname) == 0)
        {
            LIST_REMOVE(curplayer, apl_list);
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
        printf("ERROR: Player %s is not in the group.\n", charname);
        return true;
    }

    LIST_REMOVE(player, apl_list);

    aion_player_release(player);

    aion_group_dump();

    return true;
}



/*
 * Update AP value of the player's name
 */
bool aion_group_ap_update(char *charname, uint32_t apval)
{
    struct aion_player *player;

    player = aion_group_find(charname);
    if (player == NULL)
    {
        printf("ERROR: Player %s is not in the group.\n", charname);
        return false;
    }

    player->apl_apvalue += apval;

    aion_group_dump();

    return true;
}

void aion_group_dump(void)
{
    struct aion_player *curplayer;

    LIST_FOREACH(curplayer, &aion_group, apl_list)
    {
        printf("PLAYER %s: AP = %u\n", curplayer->apl_name, curplayer->apl_apvalue);
    }
}
