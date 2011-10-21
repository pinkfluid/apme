#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <pcreposix.h>

#include "regeng.h"
#include "util.h"
#include "aion.h"
#include "cmd.h"
#include "console.h"
#include "chatlog.h"
#include "event.h"

#define RE_NAME     "([0-9a-zA-Z_]+)"
#define RE_ITEM     "([0-9]+)"
#define RE_NUM_ROLL "[0-9\\.]+"

#define RE_ITEM_LOOT_SELF           100
#define RE_ITEM_LOOT_PLAYER         101

#define RE_DAMAGE_INFLICT           200
#define RE_DAMAGE_CRITICAL          201

#define RE_GROUP_SELF_JOIN          300
#define RE_GROUP_SELF_LEAVE         301
#define RE_GROUP_PLAYER_JOIN        303
#define RE_GROUP_PLAYER_LEAVE       304
#define RE_GROUP_PLAYER_DISCONNECT  305
#define RE_GROUP_PLAYER_KICK        306
#define RE_GROUP_PLAYER_OFFLINE     307
#define RE_GROUP_DISBAND            310

#define RE_CHAT_SELF                400
#define RE_CHAT_GENERAL             401
#define RE_CHAT_WHISPER             402
#define RE_CHAT_SHOUT               403

#define RE_ROLL_ITEM_SELF           500
#define RE_ROLL_ITEM_PLAYER         501
#define RE_ROLL_ITEM_PASS           502
#define RE_ROLL_ITEM_HIGHEST        503
#define RE_ROLL_DICE_SELF           504 /* When using the /roll command */
#define RE_ROLL_DICE_PLAYER         505 /* When using the /roll command */

static FILE* chatlog_file = NULL;

static bool chatlog_open(void);
static re_callback_t chatlog_parse;

struct regeng re_aion[] =
{
    /* Put damage meter at the beginning, because Aion generates a lot of text with this, so it's best they match first */
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

void parse_action_loot_item(char *player, uint32_t itemid)
{
    aion_group_loot(player, itemid);
}

void parse_action_damage_inflict(char *player, char *target, char *damage, char *skill)
{
    (void)player;
    (void)target;
    (void)damage;
    (void)skill;

    //con_printf("DMG: %s -> %s: %s (%s)\n", player, target, damage, skill);
}

void parse_action_group_self_join(void)
{
    /*
     * When joining a group, forget all previously remembered members
     * which is the same as disbanding the group.
     */
    aion_group_disband();
}

void parse_action_group_self_leave(void)
{
    aion_group_disband();
}

void parse_action_group_player_join(char *who)
{
    con_printf("GROUP: %s joined the group.\n", who);
    aion_group_join(who);
}

void parse_action_group_player_leave(char *who)
{
    con_printf("GROUP: %s left the group.\n", who);
    aion_group_leave(who);
}

void parse_action_chat_general(char *name, char *txt)
{
    aion_player_chat_cache(name, txt);
//    con_printf("CHAT: %s -> %s\n", name, txt);
}

void parse_action_chat_whisper(char *name, char *txt)
{
    aion_player_chat_cache(name, txt);
}

void parse_action_chat_shout(char *name, char *txt)
{
    aion_player_chat_cache(name, txt);
}

void parse_action_roll_item_self(void)
{
    //con_printf("ROLL: You rolled.\n");
}

void parse_action_roll_item_player(char *who)
{
    /*
     * Roll dices can be detected only for group members. So parsing rolling or passing
     * of a dice is a good way of detecting group members.
     */
    aion_group_join(who);
}

void parse_action_roll_item_pass(char *who)
{
    /* See parse_action_roll_item_player() */
    aion_group_join(who);
}

void parse_action_roll_item_highest(char *who)
{
    char aprolls[CHATLOG_CHAT_SZ];
    /*
     * Mark this user as having full inventory. If the user doesn't have a full inv
     * This flag will be cleared as soon as an item is looted.
     */
    aion_invfull_set(who, true);

    /* Update the clipboard with the new status */
    aion_group_get_aplootrights(aprolls, sizeof(aprolls));
    aion_clipboard_set(aprolls);
    event_signal(EVENT_AION_LOOT_RIGHTS);
}

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
            parse_action_group_self_join();
            break;

        case RE_GROUP_SELF_LEAVE:
        case RE_GROUP_DISBAND:
            parse_action_group_self_leave();
            break;

        case RE_GROUP_PLAYER_JOIN:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_player_join(name);
            break;

        case RE_GROUP_PLAYER_DISCONNECT:
        case RE_GROUP_PLAYER_LEAVE:
        case RE_GROUP_PLAYER_KICK:
        case RE_GROUP_PLAYER_OFFLINE:
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

    chatlog_file = fopen(chatlog_path, "r");
    if (chatlog_file == NULL)
    {
        /* This can be just a temporary error */
        con_printf("Error opening chat log\n");
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

/*
 * Initialize the chat log
 */
bool chatlog_init()
{
    if (!re_init(re_aion))
    {
        con_printf("Unable to initialize the regex subsystem.\n");
        return false;
    }

    if (!chatlog_open())
    {
        con_printf("Fatal error opening chatlog file.\n");
        return false;
    }

    return true;
}

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
