#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <pcreposix.h>

#include "regeng.h"
#include "items.h"
#include "util.h"
#include "aion.h"
#include "cmd.h"
#include "console.h"

#define RE_NAME_SZ  AION_NAME_SZ
#define RE_NAME     "([0-9a-zA-Z_]+)"
#define RE_ITEM_SZ  16
#define RE_ITEM     "([0-9]+)"

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

#define RE_CHAT_GENERAL             400
#define RE_CHAT_SELF                401

#define RE_ROLL_DICE_SELF           500
#define RE_ROLL_DICE_PLAYER         501
#define RE_ROLL_DICE_PASS           502
#define RE_ROLL_DICE_HIGHEST        503

struct regeng re_aion[] =
{
    /* Put damage meter at the beginning, because Aion generates a lot of text with this, so it's best they match first */
    {
        .re_id  = RE_DAMAGE_INFLICT,
        .re_exp = ": " RE_NAME " inflicted ([0-9.]+) damage on ([A-Za-z ]+) by using ([A-Za-z ]+)\\.",
    },
    {
        .re_id  = RE_DAMAGE_CRITICAL,
        .re_exp = ": Critical Hit! You inflicted ([0-9.]+) critical damage on ([A-Za-z ]+)\\.",
    },
    {
        .re_id  = RE_ITEM_LOOT_SELF,
        .re_exp = "You have acquired \\[item:" RE_ITEM "\\]",
    },
    {
        .re_id  = RE_ITEM_LOOT_PLAYER,
        .re_exp = RE_NAME " has acquired \\[item:" RE_ITEM "\\]",
    },
    {
        .re_id  = RE_GROUP_SELF_JOIN,
        .re_exp = "You have joined the group\\.",
    },
    {
        .re_id  = RE_GROUP_SELF_LEAVE,
        .re_exp = "You left the group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_JOIN,
        .re_exp = ": " RE_NAME " has joined your group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_LEAVE,
        .re_exp = ": " RE_NAME " has left your group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_DISCONNECT,
        .re_exp = ": " RE_NAME " has been disconnected\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_KICK,
        .re_exp = ": " RE_NAME " has been kicked out of your group\\.",
    },
    {
        .re_id  = RE_GROUP_PLAYER_OFFLINE,
        .re_exp = ": " RE_NAME " has been offline for too long and is automatically excluded from the group\\.",
    },
    {
        .re_id  = RE_GROUP_DISBAND,
        .re_exp = "The group has been disbanded\\.",
    },
    {
        .re_id  = RE_CHAT_GENERAL,
        .re_exp = ": \\[charname:" RE_NAME ";.*\\]: (.*)$",
    },
#if 0
    /* XXX Chat self is not reliable, disabling for the moment. */
    {
        .re_id  = RE_CHAT_SELF,
        .re_exp = ": " RE_NAME ": (.*)$",
    },
#endif
    {
        .re_id  = RE_ROLL_DICE_SELF,
        .re_exp = ": You rolled the dice and got a [0-9]+ \\(max\\. [0-9]+\\)\\.",
    },
    {
        .re_id  = RE_ROLL_DICE_PLAYER,
        .re_exp = ": " RE_NAME " rolled the dice and got [0-9]+ \\(max\\. [0-9]+\\)\\.",
    },
    {
        .re_id  = RE_ROLL_DICE_PASS,
        .re_exp = ": " RE_NAME " gave up rolling the dice",
    },
    {
        .re_id  = RE_ROLL_DICE_HIGHEST,
        .re_exp = ": " RE_NAME " rolled the highest",
    },
};

void parse_action_loot_item(char *player, uint32_t itemid)
{
    struct item *item;
   
    /* If we see a player's loot, he is in the group. */
    aion_group_join(player);
    /* Mark the inventory as not full anymore */
    aion_group_invfull_set(player, false);

    item = item_find(itemid);
    if (item != NULL)
    {
        if (item->item_ap != 0)
        {
            char aprolls[256];

            aion_group_apvalue_update(player, item->item_ap);
            
            /* automatically paste the new roll rights to the clipboard */
            aion_group_get_aprollrights(aprolls, sizeof(aprolls));
            clipboard_set_text(aprolls);
        }

        con_printf("LOOT: %s -> %s (%u AP)\n", player, item->item_name, item->item_ap);
    }
    else
    {
        con_printf("LOOT: %s -> %u\n", player, itemid);
    }
}

void parse_action_damage_inflict(char *player, char *target, char *damage, char *skill)
{
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

void parse_action_roll_dice_self(void)
{
    //con_printf("ROLL: You rolled.\n");
}

void parse_action_roll_dice_player(char *who)
{
    /*
     * Roll dices can be detected only for group members. So parsing rolling or passing
     * of a dice is a good way of detecting group members.
     */
    aion_group_join(who);
}

void parse_action_roll_dice_pass(char *who)
{
    /* See parse_action_roll_dice_player() */
    aion_group_join(who);
}

void parse_action_roll_dice_highest(char *who)
{
    /*
     * Mark this user as having full inventory. If the user doesn't have a full inv
     * This flag will be cleared as soon as an item is looted.
     */
    aion_group_invfull_set(who, true);
}

int parse_process(uint32_t re_id, const char* matchstr, regmatch_t *rematch, uint32_t rematch_num)
{
    char item[RE_ITEM_SZ];
    char name[RE_NAME_SZ];
    char damage[16];
    char target[RE_NAME_SZ];
    char skill[RE_NAME_SZ];
    char chat[AION_CHAT_SZ];

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

        case RE_CHAT_SELF:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);
            /* XXX: Chat self is not reliable, since it records stuff like NPC messages and Tips */
            //parse_action_chat_general(AION_NAME_DEFAULT, chat);
            break;

        case RE_ROLL_DICE_SELF:
            parse_action_roll_dice_self();
            break;

        case RE_ROLL_DICE_PLAYER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_dice_player(name);
            break;

        case RE_ROLL_DICE_PASS:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_dice_pass(name);
            break;

        case RE_ROLL_DICE_HIGHEST:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_dice_highest(name);
            break;

        default:
            con_printf("Unknown RP ID %u\n", re_id);
            break;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;
    int ii;

    con_init();

    if (!aion_init())
    {
        con_printf("Unable to initialize the Aion subsystem.\n");
        return 0;
    }

    for (ii = 0; ii < sizeof(re_aion) / sizeof(re_aion[0]); ii++)
    {
        retval = regcomp(&re_aion[ii].re_comp, re_aion[ii].re_exp, REG_EXTENDED);
        if (retval != 0)
        {
            char errstr[64];

            regerror(retval, &re_aion[ii].re_comp, errstr, sizeof(errstr));
            con_printf("Error parsing regex: %s (%s)\n", re_aion[ii].re_exp, errstr);
            return 0;
        }
    }

    {
        FILE *f;
        regmatch_t rematch[16];
        char buf[AION_CHAT_SZ];

#ifdef SYS_WINDOWS
        char *install_path = NULL;

        install_path = aion_get_install_path();
        if (install_path == NULL)
        {
            con_printf("Unable to find Aion install path.\n");
            return 1;
        }

        buf[0] = '\0';
        util_strlcat(buf, install_path, sizeof(buf));
        util_strlcat(buf, "\\Chat.log", sizeof(buf));

        con_printf("Opening chat file '%s'\n", buf);
        f = fopen(buf, "r");
        if (f == NULL)
        {
            con_printf("Error opening file\n");
            return 1;
        }

        // Seek to the end of file
        fseek(f, 0, SEEK_END);
#else
        f = fopen("./Chat.log", "r");
        if (f == NULL)
        {
            con_printf("Unable to open ./Chat.log");
            return 1;
        }
#endif

        for (;;)
        {
            cmd_poll();

            if (fgets(buf, sizeof(buf), f) == NULL)
            {
#ifdef SYS_WINDOWS
                usleep(300);
                continue;
#else
                break;
#endif
            }

            util_chomp(buf);

            for (ii = 0; ii < sizeof(re_aion) / sizeof(re_aion[0]); ii++)
            {
                if (regexec(&re_aion[ii].re_comp, buf, 16, rematch, 0) == 0)
                {
                    parse_process(re_aion[ii].re_id, buf, rematch, 16);
                }
            }
        }
    }

    return 0;
}

