/*
 * chatlog.c - APme: Aion Automatic Abyss Point Tracker
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
 * Aion chatlog parser
 * 
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <pcreposix.h>

#include "regeng.h"
#include "util.h"
#include "aion.h"
#include "cmd.h"
#include "console.h"
#include "chatlog.h"
#include "event.h"

/**
 * @defgroup chatlog Aion Chatlog Parser
 *
 * @brief This module parses the chatlog
 * 
 * This module reads the chatlog line-by-line and matches
 * each line against a regex and fires off an event accordingly.
 * 
 * Events are processed in a giant loop switch. The loop switch
 * extracts some information from the regex and calls a function
 * with the information as parameters.
 * 
 * @{
 */

#define RE_NAME     "([0-9a-zA-Z_]+)"           /**< Character name regex pattern           */
#define RE_ITEM     "([0-9]+)"                  /**< Item number regex pattern              */
#define RE_NUM_ROLL "[0-9\\.]+"                 /**< Item link regex pattern                */

#define RE_ITEM_LOOT_SELF           100         /**< Event, item looted by player           */
#define RE_ITEM_LOOT_PLAYER         101         /**< Event, item looted by a character      */

#define RE_DAMAGE_INFLICT           200         /**< Damage was inflicted                   */
#define RE_DAMAGE_CRITICAL          201         /**< Damage was inflicted and was critical  */

#define RE_GROUP_SELF_JOIN          300         /**< The player joined a group              */
#define RE_GROUP_SELF_LEAVE         301         /**< The player left the group              */
#define RE_GROUP_PLAYER_JOIN        303         /**< Some other player joined the group     */
#define RE_GROUP_PLAYER_LEAVE       304         /**< Some other player left the group       */
#define RE_GROUP_PLAYER_DISCONNECT  305         /**< Some player was disconnected           */
#define RE_GROUP_PLAYER_KICK        306         /**< A player was kicked from the group     */
#define RE_GROUP_PLAYER_OFFLINE     307         /**< A player in the group went offline     */
#define RE_GROUP_DISBAND            308         /**< The group was disbanded                */

#define RE_ALI_SELF_JOIN            350         /**< The player joined a group              */
#define RE_ALI_SELF_LEAVE           351         /**< The player left the group              */
#define RE_ALI_PLAYER_JOIN          353         /**< Some other player joined the group     */
#define RE_ALI_PLAYER_LEAVE         354         /**< Some other player left the group       */
#define RE_ALI_PLAYER_DISCONNECT    355         /**< Some player was disconnected           */
#define RE_ALI_PLAYER_KICK          356         /**< A player was kicked from the group     */
#define RE_ALI_PLAYER_OFFLINE       357         /**< A player in the group went offline     */
#define RE_ALI_DISBAND              358         /**< The group was disbanded                */

#define RE_CHAT_SELF                400         /**< Chat from the player itself            */
#define RE_CHAT_GENERAL             401         /**< General chat                           */
#define RE_CHAT_WHISPER             402         /**< A whisper was received                 */
#define RE_CHAT_SHOUT               403         /**< Somebody shouteed something            */

#define RE_ROLL_ITEM_SELF           500         /**< The player rolled on an item           */
#define RE_ROLL_ITEM_PLAYER         501         /**< Some other player rolled on an item    */
#define RE_ROLL_ITEM_PASS           502         /**< The player passed on an item           */
#define RE_ROLL_ITEM_HIGHEST        503         /**< Somebody rolled the highest            */
#define RE_ROLL_DICE_SELF           504         /**< The player used /roll to roll a dice   */
#define RE_ROLL_DICE_PLAYER         505         /**< Group member used /roll to roll a dice */

static FILE* chatlog_file = NULL;   /**< Chatlog FILE descriptor            */

static bool chatlog_open(void); 
static re_callback_t chatlog_parse; /**< Declaration of chatlog_parse()     */

/**
 * Define chatlog regex patterns and corresponding event.
 *
 * This list is scanned from beginning to end. If a regular expressions matches,
 * an event is generated.
 *
 * For speed reasons, try to put most commonly matched regular expressions at the
 * beginning
 *
 * @see regeng
 *
 * @showinitializer
 */
struct regeng re_aion[] =
{
    {
        .re_id  = RE_DAMAGE_INFLICT,
        .re_exp = "^: " RE_NAME " inflicted ([0-9.]+) damage on ([A-Za-z ]+) by using ([A-Za-z ]+)\\.",
    },
    {
        .re_id  = RE_DAMAGE_CRITICAL,
        .re_exp = "^: Critical Hit! You inflicted ([0-9.]+) critical damage on ([A-Za-z ]+)\\.",
    },
    {
        .re_id  = RE_ITEM_LOOT_SELF,
        .re_exp = "^: You have acquired \\[item:" RE_ITEM "\\]",
    },
    {
        .re_id  = RE_ITEM_LOOT_PLAYER,
        .re_exp = "^: " RE_NAME " has acquired \\[item:" RE_ITEM "\\]",
    },
    {
        .re_id  = RE_GROUP_SELF_JOIN,
        .re_exp = "^: You have joined the group\\.",
    },
    {
        .re_id  = RE_GROUP_SELF_LEAVE,
        .re_exp = "^: You left the group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_JOIN,
        .re_exp = "^: " RE_NAME " has joined your group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_LEAVE,
        .re_exp = "^: " RE_NAME " has left your group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_DISCONNECT,
        .re_exp = "^: " RE_NAME " has been disconnected\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_KICK,
        .re_exp = "^: " RE_NAME " has been kicked out of your group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_OFFLINE,
        .re_exp = "^: " RE_NAME " has been offline for too long and is automatically excluded from the group\\.",
    },
    {
        .re_id  = RE_GROUP_DISBAND,
        .re_exp = "^: The group has been disbanded\\.",
    },
    {
        .re_id  = RE_ALI_SELF_JOIN,
        .re_exp = "^: You have joined the alliance\\.",
    },
    {
        .re_id  = RE_ALI_SELF_LEAVE,
        .re_exp = "^: You have left the alliance\\.",
    },
    {
        .re_id  = RE_ALI_PLAYER_JOIN,
        .re_exp = "^: " RE_NAME " has joined the alliance\\.",
    },
    {
        .re_id  = RE_ALI_PLAYER_LEAVE,
        .re_exp = "^: " RE_NAME " has left the alliance\\.",
    },
    {
        .re_id  = RE_ALI_PLAYER_KICK,
        .re_exp = "^: " RE_NAME " has been kicked out of the alliance\\.",
    },
    {
        .re_id  = RE_ALI_PLAYER_OFFLINE,
        .re_exp = "^: " RE_NAME " has been offline for too long and had been automatically kicked out of the alliance\\."
    },
    {
        .re_id  = RE_ALI_DISBAND,
        .re_exp = "^: The alliance has been disbanded\\.",
    },
    {
        .re_id  = RE_CHAT_GENERAL,
        .re_exp = "^: \\[charname:" RE_NAME ";.*\\]: (.*)$",
    },
    {
        .re_id  = RE_CHAT_WHISPER,
        .re_exp = "^: \\[charname:" RE_NAME ";.*\\] Whispers: (.*)$",
    },
    {
        .re_id  = RE_CHAT_SHOUT,
        .re_exp = "^: \\[charname:" RE_NAME ";.*\\] Shouts: (.*)$",
    },
#if 0
    /* XXX Chat self is not reliable, disabling for the moment. */
    {
        .re_id  = RE_CHAT_SELF,
        .re_exp = "^: " RE_NAME ": (.*)$",
    },
#endif
    {
        .re_id  = RE_ROLL_ITEM_SELF,
        .re_exp = "^: You rolled the dice and got " RE_NUM_ROLL " \\(max\\. " RE_NUM_ROLL "\\)\\.",
    },
    {
        .re_id  = RE_ROLL_ITEM_PLAYER,
        .re_exp = "^: " RE_NAME " rolled the dice and got " RE_NUM_ROLL " \\(max\\. " RE_NUM_ROLL "\\)\\.",
    },
    {
        .re_id  = RE_ROLL_ITEM_PASS,
        .re_exp = "^: " RE_NAME " gave up rolling the dice",
    },
    {
        .re_id  = RE_ROLL_ITEM_HIGHEST,
        .re_exp = "^: " RE_NAME " rolled the highest",
    },
    {
        /* The onlly difference between this and RE_ROLL_ITEM_SELF is in the "got a" vs "got" text */
        .re_id  = RE_ROLL_DICE_SELF,
        .re_exp = "^: You rolled the dice and got a " RE_NUM_ROLL " \\(max\\. " RE_NUM_ROLL "\\)\\.",

    },
    {
        /* The onlly difference between this and RE_ROLL_ITEM_PLAYER is in the "got a" vs "got" text */
        .re_id  = RE_ROLL_DICE_PLAYER,
        .re_exp = "^: " RE_NAME " rolled the dice and got a " RE_NUM_ROLL " \\(max\\. " RE_NUM_ROLL "\\)\\.",
    },

    RE_REGENG_END
};

/**
 * @name Chatlog Event Processing Functions
 *
 * These process various events that are triggered by the regeng
 * engine when it parses the chat log.
 *
 * They process stuff like:
 *  - Item loot
 *  - Item rolls
 *  - Damage taken/received
 *  - Group joins/leaves
 *  - Probably much more..
 *
 * @{
 */

/**
 * Processes an "item loot" event. This function is called
 * whenever a player or a group/alliance member loots 
 * an item
 *
 * Triggered by:
 *  - RE_ITEM_LOOT_SELF
 *  - RE_ITEM_LOOT_PLAYER
 *
 * @param[in]       player      Character name
 * @param[in]       itemid      Item id number
 *
 */
void parse_action_loot_item(char *player, uint32_t itemid)
{
    aion_group_loot(player, itemid);
}

/** 
 * Process a "damage inflict" event. This function is called
 * whenever something in chat range inflicts damage
 *
 * Triggered by:
 *      - RE_DAMAGE_INFLICT
 *      - RE_DAMAGE_CRITICAL
 *
 * @param[in]       player      Character name
 * @param[in]       target      Afflicted target name
 * @param[in]       damage      Damage number (may contain ,.)
 * @param[in]       skill       Skill that was used to inflict damage, or NULL if none
 *
 */
void parse_action_damage_inflict(char *player, char *target, char *damage, char *skill)
{
    (void)player;
    (void)target;
    (void)damage;
    (void)skill;

    //con_printf("DMG: %s -> %s: %s (%s)\n", player, target, damage, skill);
}

/**
 * Process a "self join group" event. This function is called
 * when the player joins a group.
 *
 * Triggered by:
 *      - RE_GROUP_SELF_JOIN
 *
 */
void parse_action_group_self_join(void)
{
    /*
     * When joining a group, forget all previously remembered members
     * which is the same as disbanding the group.
     */
    aion_group_disband();
}

/**
 * Process a "self leave group" event. This function is called
 * when the player leaves a group or the group is disbanded.
 *
 * Triggered by:
 *
 *      - RE_GROUP_SELF_LEAVE
 *      - RE_GROUP_DISBAND
 */
void parse_action_group_self_leave(void)
{
    aion_group_disband();
}

/**
 * Process "player join group" event. This is triggered
 * when another player joins a group/alliance.
 * 
 * Triggered by:
 *      - RE_GROUP_PLAYER_JOIN:
 * 
 * @param[in]       who     Character name that joined
 */ 
void parse_action_group_player_join(char *who)
{
    con_printf("GROUP: %s joined the group.\n", who);
    aion_group_join(who);
}

/**
 * Process a "player leave group" event. This is tirggered
 * when another player leaves a group/alliance.
 *
 * This even is triggered by lots of events, like
 * disconnects, group leavse, offline disconnects...
 *
 * Triggered by:
 *      - RE_GROUP_PLAYER_DISCONNECT
 *      - RE_GROUP_PLAYER_LEAVE
 *      - RE_GROUP_PLAYER_KICK
 *      - RE_GROUP_PLAYER_OFFLINE
 *
 * @param[in]       who     Character name that left the group
 */
void parse_action_group_player_leave(char *who)
{
    con_printf("GROUP: %s left the group.\n", who);
    aion_group_leave(who);
}

/**
 * Process a "general chat" event. This function is called
 * whenever other players say something on on /say,
 * /group, /legion ...
 *
 * @note /1,/2,/3 chat is not part of this
 *
 * Triggered by:
 *      - RE_CHAT_GENERAL
 *
 * @param[in]       name        The player that said something on chat
 * @param[in]       txt         The chat line
 *
 */
void parse_action_chat_general(char *name, char *txt)
{
    aion_player_chat_cache(name, txt);
//    con_printf("CHAT: %s -> %s\n", name, txt);
}

/**
 * Process a "whisper chat" event. This function is called
 * whenever the player receives a whisper.
 *
 * Triggered by:
 *      - RE_CHAT_WHISPER
 *
 * @param[in]       name        The whispering player
 * @param[in]       txt         The whisper chat line
 *
 */
void parse_action_chat_whisper(char *name, char *txt)
{
    aion_player_chat_cache(name, txt);
}

/**
 * Process a "shout chat" event. This function is called
 * whenever the player receives a shout from another 
 * player. The player obviously must be in range of
 * the shout.
 *
 * Triggered by
 *      - RE_CHAT_SHOUT
 *
 * @param[in]       name        The player that shouted
 * @param[in]       txt         The shout chat line
 *
 */
void parse_action_chat_shout(char *name, char *txt)
{
    aion_player_chat_cache(name, txt);
}

/**
 * Process a "self rolled on item" event. This function
 * is called whenever a player rolls for an item.
 *
 * Triggered by:
 *      - RE_ROLL_ITEM_SELF
 *
 * @note This is currently unused.
 *
 */
void parse_action_roll_item_self(void)
{
    //con_printf("ROLL: You rolled.\n");
}

/**
 * Process a "player rolled on item" event. This function
 * is called whenever a player rolls on an item.
 *
 * This is also used to auto-detect group members.
 * the problem is that there's no information in the chat log
 * of group members when you join an EXISTING group. 
 * For the time being, the best way to detect the other
 * players is to watch for this event.
 *
 * Triggered by:
 *      - RE_ROLL_ITEM_PLAYER
 *
 * @param[in]       who         Player that rolled on an item
 */
void parse_action_roll_item_player(char *who)
{
    aion_group_join(who);
}

/**
 * Process a "player passed on item" event. This function is called
 * whenever a player passed on an item.
 *
 * This is also used to autodetect group members. Refer to @ref
 * parse_action_roll_item_player() for more info.
 * 
 * Triggered by:
 *      - RE_ROLL_ITEM_PASS
 *
 * @param[in]       who         Player that passed on an item
 */
void parse_action_roll_item_pass(char *who)
{
    /* See parse_action_roll_item_player() */
    aion_group_join(who);
}

/**
 * Process the "player rolled the highest on an item" event.
 * This function is called whenever a player wins an item
 * by rolling the highest dice.
 *
 * This event is used to detect an "inventory full" condition.
 * If the player wins an item, but doesn't loot it, it means
 * his inventory is full.
 * 
 * Triggered by:
 *      - RE_ROLL_ITEM_HIGHEST
 *
 * @param[in]       who         Player who won the item
 *
 */
void parse_action_roll_item_highest(char *who)
{
    char aprolls[CHATLOG_CHAT_SZ];
    /*
     * Mark this user as having full inventory.
     * This flag will be cleared as soon as an item is looted.
     */
    aion_invfull_set(who, true);

    /* Update the clipboard with the new status */
    aion_aploot_rights(aprolls, sizeof(aprolls));
    aion_clipboard_set(aprolls);
    event_signal(EVENT_AION_LOOT_RIGHTS);
}

/**
 * @}
 */

/**
 * This is the gigantic switch case that maps the matched ID
 * to an event function. What parameters are passed to the
 * handler is determined in here.
 *
 * @param[in]       re_id           Matched regex ID
 * @param[in]       matchstr        Matched string
 * @param[in]       rematch         regmatch_t regex, used to extract arguments
 * @param[in]       rematch_num     Number of regmatch_t structures in <I>rematch</I>
 */
void chatlog_parse(uint32_t re_id, const char* matchstr, regmatch_t *rematch, size_t rematch_num)
{
    char item[CHATLOG_ITEM_SZ];
    char name[CHATLOG_NAME_SZ];
    char damage[CHATLOG_NAME_SZ];
    char target[CHATLOG_NAME_SZ];
    char skill[CHATLOG_NAME_SZ];
    char chat[CHATLOG_CHAT_SZ];

    (void)rematch_num;

    switch (re_id)
    {
        case RE_ITEM_LOOT_SELF:
            re_strlcpy(item, matchstr, sizeof(item), rematch[1]);
            parse_action_loot_item(AION_NAME_DEFAULT, strtoul(item, NULL, 10));
            break;

        case RE_ITEM_LOOT_PLAYER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(item, matchstr, sizeof(item), rematch[2]);

            parse_action_loot_item(name, strtoul(item, NULL, 10));
            break;

        case RE_DAMAGE_INFLICT:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(damage, matchstr, sizeof(damage), rematch[2]);
            re_strlcpy(target, matchstr, sizeof(target), rematch[3]);
            re_strlcpy(skill, matchstr, sizeof(skill), rematch[4]);

            parse_action_damage_inflict(name, target, damage, skill);
            break;

        case RE_DAMAGE_CRITICAL:
            re_strlcpy(damage, matchstr, sizeof(damage), rematch[1]);
            re_strlcpy(target, matchstr, sizeof(target), rematch[2]);

            parse_action_damage_inflict(AION_NAME_DEFAULT, target, damage, "Critical");
            break;

        case RE_GROUP_SELF_JOIN:
        case RE_ALI_SELF_JOIN:
            parse_action_group_self_join();
            break;

        case RE_GROUP_SELF_LEAVE:
        case RE_GROUP_DISBAND:
        case RE_ALI_SELF_LEAVE:
        case RE_ALI_DISBAND:
            parse_action_group_self_leave();
            break;

        case RE_GROUP_PLAYER_JOIN:
        case RE_ALI_PLAYER_JOIN:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_player_join(name);
            break;

        case RE_GROUP_PLAYER_DISCONNECT:
        case RE_GROUP_PLAYER_LEAVE:
        case RE_GROUP_PLAYER_KICK:
        case RE_GROUP_PLAYER_OFFLINE:
        case RE_ALI_PLAYER_DISCONNECT:
        case RE_ALI_PLAYER_LEAVE:
        case RE_ALI_PLAYER_KICK:
        case RE_ALI_PLAYER_OFFLINE:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_player_leave(name);
            break;

        case RE_CHAT_GENERAL:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);

            parse_action_chat_general(name, chat);
            break;

        case RE_CHAT_WHISPER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);

            parse_action_chat_whisper(name, chat);
            break;

        case RE_CHAT_SHOUT:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);

            parse_action_chat_shout(name, chat);
            break;

        case RE_CHAT_SELF:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);
            /* XXX: Chat self is not reliable, since it records stuff like NPC messages and Tips */
            //parse_action_chat_general(AION_NAME_DEFAULT, chat);
            break;

        case RE_ROLL_ITEM_SELF:
            parse_action_roll_item_self();
            break;

        case RE_ROLL_ITEM_PLAYER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_item_player(name);
            break;

        case RE_ROLL_ITEM_PASS:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_item_pass(name);
            break;

        case RE_ROLL_ITEM_HIGHEST:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_item_highest(name);
            break;

        case RE_ROLL_DICE_SELF:
        case RE_ROLL_DICE_PLAYER:
            /* Nothing to do here, these trigger if the user types /roll */
            break;

        default:
            con_printf("Unknown RP ID %u\n", re_id);
            break;
    }
}

/**
 * Open the chatlog file
 * 
 * Uses aion_default_install_path() to find the path to the chatlog file.
 *
 * @retval      true        If successfull
 * @retval      false       On error
 */ 
bool chatlog_open(void)
{
    char *chatlog_dir;
    char chatlog_path[1024];

    if (chatlog_file != NULL)
    {
        /* Chat log file alrady open, return */
        return true;
    }

#ifdef SYS_WINDOWS
    /* Try to open the cthatlog file */
    chatlog_dir = aion_default_install_path();
    if (chatlog_dir == NULL)
    {
        con_printf("FATAL: Unable to find Aion install path.\n");
        return false;
    }

    /* Construct the path */
    util_strlcpy(chatlog_path, chatlog_dir, sizeof(chatlog_path));
    util_strlcat(chatlog_path, "\\", sizeof(chatlog_path));
    util_strlcat(chatlog_path, CHATLOG_FILENAME, sizeof(chatlog_path));
#else
    (void)chatlog_dir;
    util_strlcpy(chatlog_path, "./Chat.log", sizeof(chatlog_path));
#endif

    chatlog_file = sys_fopen_force(chatlog_path, "r");
    if (chatlog_file == NULL)
    {
        /* This can be just a temporary error, so return success */
        con_printf("Error opening chat log: %s\n", chatlog_path);
        return true;
    }

#ifdef SYS_WINDOWS
    // Seek to the end of file
    if (fseek(chatlog_file, 0, SEEK_END) != 0)
    {
        /* If we didn't succeed in the seek, we might be in trouble, so return a hard error */
        return false;
    }
#endif

    return true;
}

/**
 * Initialize the chat log facility
 *
 * This must be called before any other chatlog function
 *
 * @retval      true        On success
 * @retval      false       On error
 */
bool chatlog_init()
{
    if (!re_init(re_aion))
    {
        con_printf("Unable to initialize the regex subsystem.\n");
        return false;
    }

    return true;
}

/**
 * Processes a line from the chatlog
 *
 * Calls the regeng re_parse() function, which is used to 
 * match a particular line to an event
 *
 * @param       chatstr     Chat line to process
 *
 * @retval      true        On sucess  
 * @retval      false       If invalid chatlog line
 */
bool chatlog_readstr(char *chatstr)
{
    char *pchat; 
    /* Skip the timestamp, check if we're at the ':' character */
    pchat = chatstr + CHATLOG_PREFIX_LEN;

    /* Clearly this is an invalid line */
    if (*pchat != ':')
    { 
        return true;
    }

    return re_parse(chatlog_parse, re_aion, pchat);
}

/**
 * Checks if there are any new lines in the chatlog.
 * If there are it reads the chatlog and processes
 * the line; otherwise it immediatelly returns
 *
 * @retval      true        On success (note, this is returned
 *                          even if there are no new lines)
 * @retval      false       If fatal error
 */
bool chatlog_poll()
{
    char chatstr[CHATLOG_CHAT_SZ];

    if (!chatlog_open())
    {
        return false;
    }

    /* Chatlog not open yet, return so we might process it later */
    if (chatlog_file == NULL) return true;

    /* Nothing to read? */
    while (fgets(chatstr, sizeof(chatstr), chatlog_file) != NULL)
    {
        /* Remove ending new-lines */
        util_chomp(chatstr);
        chatlog_readstr(chatstr);
    } 

    return true;
}

/**
 * This reads the file <I>file</I> as if it was a chatlog
 *
 * This is mainly used for debugging.
 *
 * @param[in]       file        File to read chastlog from
 *
 * @retval          true        On success
 * @retval          false       If there was an error processing the file
 *                              or could not be found
 */
bool chatlog_readfile(char *file)
{
    char chatstr[CHATLOG_CHAT_SZ];
    FILE *chatfile;

    chatfile = fopen(file, "r");
    if (chatfile == NULL)
    {
        con_printf("CHATLOG: Error reading file %s\n", file);
        return false;
    }

    /* Nothing to read? */
    while (fgets(chatstr, sizeof(chatstr), chatfile) != NULL)
    {
        /* Remove ending new-lines */
        util_chomp(chatstr);

        /* Parse chatlog */
        chatlog_readstr(chatstr);
    } 

    fclose(chatfile);

    return true;
}

/**
 * @}
 */

