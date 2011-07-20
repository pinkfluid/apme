/* Thanks to Charisse for testing :) */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <pcre/pcreposix.h>

#include "items.h"
#include "util.h"
#include "aion.h"
#include "cmd.h"
#include "console.h"

#define REGEX_NAME_SZ   AION_NAME_SZ
#define REGEX_NAME      "([0-9a-zA-Z_]+)"
#define REGEX_ITEM_SZ   16
#define REGEX_ITEM      "([0-9]+)"

#define RP_ITEM_LOOT_SELF           100
#define RP_ITEM_LOOT_PLAYER         101

#define RP_DAMAGE_INFLICT           200
#define RP_DAMAGE_CRITICAL          201

#define RP_GROUP_SELF_JOIN          300
#define RP_GROUP_SELF_LEAVE         301
#define RP_GROUP_PLAYER_JOIN        303
#define RP_GROUP_PLAYER_LEAVE       304
#define RP_GROUP_PLAYER_DISCONNECT  305
#define RP_GROUP_PLAYER_KICK        306
#define RP_GROUP_PLAYER_OFFLINE     307
#define RP_GROUP_DISBAND            310

#define RP_CHAT_GENERAL             400
#define RP_CHAT_SELF                401

#define RP_ROLL_DICE_SELF           500
#define RP_ROLL_DICE_PLAYER         501
#define RP_ROLL_DICE_PASS           502

struct regex_parse
{
    uint32_t    rp_id;
    regex_t     rp_comp;
    char        rp_exp[256];
};

struct regex_parse rp_aion[] =
{
    /* Put damage meter at the beginning, because Aion generates a lot of text with this, so it's best they match first */
    {
        .rp_id  = RP_DAMAGE_INFLICT,
        .rp_exp = ": " REGEX_NAME " inflicted ([0-9.]+) damage on ([A-Za-z ]+) by using ([A-Za-z ]+)\\.",
    },
    {
        .rp_id  = RP_DAMAGE_CRITICAL,
        .rp_exp = ": Critical Hit! You inflicted ([0-9.]+) critical damage on ([A-Za-z ]+)\\.",
    },
    {
        .rp_id  = RP_ITEM_LOOT_SELF,
        .rp_exp = "You have acquired \\[item:" REGEX_ITEM "\\]",
    },
    {
        .rp_id  = RP_ITEM_LOOT_PLAYER,
        .rp_exp = REGEX_NAME " has acquired \\[item:" REGEX_ITEM "\\]",
    },
    {
        .rp_id  = RP_GROUP_SELF_JOIN,
        .rp_exp = "You have joined the group\\.",
    },
    {
        .rp_id  = RP_GROUP_SELF_LEAVE,
        .rp_exp = "You left the group\\.",
    },
    {
        .rp_id  = RP_GROUP_PLAYER_JOIN,
        .rp_exp = ": " REGEX_NAME " has joined your group\\.",
    },
    {
        .rp_id  = RP_GROUP_PLAYER_LEAVE,
        .rp_exp = ": " REGEX_NAME " has left your group\\.",
    },
    {
        .rp_id  = RP_GROUP_PLAYER_DISCONNECT,
        .rp_exp = ": " REGEX_NAME " has been disconnected\\.",
    },
    {
        .rp_id  = RP_GROUP_PLAYER_KICK,
        .rp_exp = ": " REGEX_NAME " has been kicked out of your group\\.",
    },
    {
        .rp_id  = RP_GROUP_PLAYER_OFFLINE,
        .rp_exp = ": " REGEX_NAME " has been offline for too long and is automatically excluded from the group\\.",
    },
    {
        .rp_id  = RP_GROUP_DISBAND,
        .rp_exp = "The group has been disbanded\\.",
    },
    {
        .rp_id  = RP_CHAT_GENERAL,
        .rp_exp = ": \\[charname:" REGEX_NAME ";.*\\]: (.*)$",
    },
    {
        .rp_id  = RP_CHAT_SELF,
        .rp_exp = ": " REGEX_NAME ": (.*)$",
    },
    {
        .rp_id  = RP_ROLL_DICE_SELF,
        .rp_exp = ": You rolled the dice and got a [0-9]+ \\(max\\. [0-9]+\\)\\.",
    },
    {
        .rp_id  = RP_ROLL_DICE_PLAYER,
        .rp_exp = ": " REGEX_NAME " rolled the dice and got [0-9]+ \\(max\\. [0-9]+\\)\\.",
    },
    {
        .rp_id  = RP_ROLL_DICE_PASS,
        .rp_exp = ": " REGEX_NAME " gave up rolling the dice.",
    },
};

void parse_action_loot_item(char *player, uint32_t itemid)
{
    struct item *item;
   
    /* If we see a player's loot, he is in the group. */
    aion_group_join(player);

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


int parse_process(uint32_t rp_id, const char* matchstr, regmatch_t *rematch, uint32_t rematch_num)
{
    char item[REGEX_ITEM_SZ];
    char name[REGEX_NAME_SZ];
    char damage[16];
    char target[REGEX_NAME_SZ];
    char skill[REGEX_NAME_SZ];
    char chat[AION_CHAT_SZ];

    switch (rp_id)
    {
        case RP_ITEM_LOOT_SELF:
            util_re_strlcpy(item, matchstr, sizeof(item), rematch[1]);
            parse_action_loot_item(AION_NAME_DEFAULT, strtoul(item, NULL, 10));
            break;

        case RP_ITEM_LOOT_PLAYER:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            util_re_strlcpy(item, matchstr, sizeof(item), rematch[2]);

            parse_action_loot_item(name, strtoul(item, NULL, 10));
            break;

        case RP_DAMAGE_INFLICT:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            util_re_strlcpy(damage, matchstr, sizeof(damage), rematch[2]);
            util_re_strlcpy(target, matchstr, sizeof(target), rematch[3]);
            util_re_strlcpy(skill, matchstr, sizeof(skill), rematch[4]);

            parse_action_damage_inflict(name, target, damage, skill);
            break;

        case RP_DAMAGE_CRITICAL:
            util_re_strlcpy(damage, matchstr, sizeof(damage), rematch[1]);
            util_re_strlcpy(target, matchstr, sizeof(target), rematch[2]);

            parse_action_damage_inflict(AION_NAME_DEFAULT, target, damage, "Critical");
            break;

        case RP_GROUP_SELF_JOIN:
            parse_action_group_self_join();
            break;

        case RP_GROUP_SELF_LEAVE:
        case RP_GROUP_DISBAND:
            parse_action_group_self_leave();
            break;

        case RP_GROUP_PLAYER_JOIN:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_player_join(name);
            break;

        case RP_GROUP_PLAYER_DISCONNECT:
        case RP_GROUP_PLAYER_LEAVE:
        case RP_GROUP_PLAYER_KICK:
        case RP_GROUP_PLAYER_OFFLINE:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_player_leave(name);
            break;

        case RP_CHAT_GENERAL:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            util_re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);

            parse_action_chat_general(name, chat);
            break;

        case RP_CHAT_SELF:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            util_re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);
            /* XXX: Chat self is not reliable, since it records stuff like NPC messages and Tips */
            //parse_action_chat_general(AION_NAME_DEFAULT, chat);
            break;

        case RP_ROLL_DICE_SELF:
            parse_action_roll_dice_self();
            break;

        case RP_ROLL_DICE_PLAYER:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_dice_player(name);
            break;

        case RP_ROLL_DICE_PASS:
            util_re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_roll_dice_pass(name);
            break;


        default:
            con_printf("Unknown RP ID %u\n", rp_id);
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

    for (ii = 0; ii < sizeof(rp_aion) / sizeof(rp_aion[0]); ii++)
    {
        retval = regcomp(&rp_aion[ii].rp_comp, rp_aion[ii].rp_exp, REG_EXTENDED);
        if (retval != 0)
        {
            char errstr[64];

            regerror(retval, &rp_aion[ii].rp_comp, errstr, sizeof(errstr));
            con_printf("Error parsing regex: %s (%s)\n", rp_aion[ii].rp_exp, errstr);
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

            for (ii = 0; ii < sizeof(rp_aion) / sizeof(rp_aion[0]); ii++)
            {
                if (regexec(&rp_aion[ii].rp_comp, buf, 16, rematch, 0) == 0)
                {
                    parse_process(rp_aion[ii].rp_id, buf, rematch, 16);
                }
            }
        }
    }

    return 0;
}

