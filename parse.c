/* Thanks to Charisse for testing :) */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "items.h"
#include "group.h"
#include "util.h"

#define REGEX_NAME_SZ   64
#define REGEX_NAME  "([0-9a-zA-Z_]+)"
#define REGEX_ITEM_SZ   16
#define REGEX_ITEM  "([0-9]+)"

#define RP_ITEM_LOOT_SELF       100
#define RP_ITEM_LOOT_PLAYER     101

#define RP_DAMAGE_INFLICT       200
#define RP_DAMAGE_CRITICAL      201

#define RP_GROUP_JOIN_SELF      300
#define RP_GROUP_LEAVE_SELF     301
#define RP_GROUP_JOIN_PLAYER    302
#define RP_GROUP_LEAVE_PLAYER   303
#define RP_GROUP_DISBAND        304

#define RP_CHAT_GENERAL         400

struct regex_parse
{
    uint32_t    rp_id;
    regex_t     rp_comp;
    char        rp_exp[256];
};

struct regex_parse rp_aion[] =
{
    {
        .rp_id  = RP_ITEM_LOOT_SELF,
        .rp_exp = "You have acquired \\[item:" REGEX_ITEM "\\]",
    },
    {
        .rp_id  = RP_ITEM_LOOT_PLAYER,
        .rp_exp = REGEX_NAME " has acquired \\[item:" REGEX_ITEM "\\]",
    },
    {
        .rp_id  = RP_DAMAGE_INFLICT,
        .rp_exp = ": " REGEX_NAME " inflicted ([0-9.]+) damage on ([A-Za-z ]+) by using ([A-Za-z ]+)\\.",
    },
    {
        .rp_id  = RP_DAMAGE_CRITICAL,
        .rp_exp = ": Critical Hit! You inflicted ([0-9.]+) critical damage on ([A-Za-z ]+)\\.",
    },
    {
        .rp_id  = RP_GROUP_JOIN_SELF,
        .rp_exp = "You have joined the group\\.",
    },
    {
        .rp_id  = RP_GROUP_LEAVE_SELF,
        .rp_exp = "You left the group\\.",
    },
    {
        .rp_id  = RP_GROUP_DISBAND,
        .rp_exp = "The group has been disbanded\\.",
    },
    {
        .rp_id  = RP_GROUP_JOIN_PLAYER,
        .rp_exp = ": " REGEX_NAME " has joined your group\\.",
    },
    {
        .rp_id  = RP_GROUP_LEAVE_PLAYER,
        .rp_exp = ": " REGEX_NAME " has left your group\\.",
    },
    {
        .rp_id  = RP_CHAT_GENERAL,
        .rp_exp = ": \\[charname:" REGEX_NAME ";.*\\]: (.*)$",
    }
};

void parse_action_loot_item(char *_player, uint32_t itemid)
{
    struct item *item;
    char *player;
   
    player = (_player == NULL) ? "You" : _player;

    item = item_find(itemid);
    if (item != NULL)
    {
        if (item->item_ap != 0)
        {
            group_ap_update(player, item->item_ap);
            group_stats();
            group_ap_eligible();
        }

        printf("LOOT: %s -> %s (%u AP)\n", player, item->item_name, item->item_ap);
    }
    else
    {
        printf("LOOT: %s -> %u\n", player, itemid);
    }
}

void parse_action_damage_inflict(char *player, char *target, char *damage, char *skill)
{
    //printf("DMG: %s -> %s: %s (%s)\n", player, target, damage, skill);
}

void parse_action_group_join(char *who)
{
    printf("GROUP: %s joined the group.\n", who);
}

void parse_action_group_leave(char *who)
{
    printf("GROUP: %s left the group.\n", who);
}

void parse_action_chat_general(char *name, char *txt)
{
    printf("CHAT: %s -> %s\n", name, txt);
}

void re_strlcpy(char *outstr, const char *instr, ssize_t outsz, regmatch_t rem)
{
    ssize_t sz = rem.rm_eo - rem.rm_so;

    /* We cannot copy 0 bytes */
    if ((outsz == 0) || (outstr == NULL))
    {
        /* I know this message makes 0 sense, but that makes it unique too */
        assert(!"Unable to copy 0 bytes to NULL");
    }

    /* Sanity checks */
    if ((rem.rm_so < 0) || 
        (rem.rm_eo < 0) ||
        (sz <= 0))
    {
        *outstr = '\0';
    }

    /* Check if the out string has enough space + the terminating NULL character */
    if ((sz + 1) > outsz)
    {
        /* Cap the size */
        sz = outsz - 1;
    }


    strncpy(outstr, instr + rem.rm_so, sz);

    /* Terminate it with NULL */
    outstr[sz] = '\0';
}

int parse_process(uint32_t rp_id, const char* matchstr, regmatch_t *rematch, uint32_t rematch_num)
{
    char item[REGEX_ITEM_SZ];
    char name[REGEX_NAME_SZ];
    char damage[16];
    char target[REGEX_NAME_SZ];
    char skill[REGEX_NAME_SZ];
    char chat[256];

    switch (rp_id)
    {
        case RP_ITEM_LOOT_SELF:
            re_strlcpy(item, matchstr, sizeof(item), rematch[1]);

            parse_action_loot_item(NULL, strtoul(item, NULL, 10));
            break;

        case RP_ITEM_LOOT_PLAYER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(item, matchstr, sizeof(item), rematch[2]);

            parse_action_loot_item(name, strtoul(item, NULL, 10));
            break;

        case RP_DAMAGE_INFLICT:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(damage, matchstr, sizeof(damage), rematch[2]);
            re_strlcpy(target, matchstr, sizeof(target), rematch[3]);
            re_strlcpy(skill, matchstr, sizeof(skill), rematch[4]);

            parse_action_damage_inflict(name, target, damage, skill);
            break;

        case RP_DAMAGE_CRITICAL:
            re_strlcpy(damage, matchstr, sizeof(damage), rematch[1]);
            re_strlcpy(target, matchstr, sizeof(target), rematch[2]);

            parse_action_damage_inflict("You", target, damage, "Critical");
            break;

        case RP_GROUP_JOIN_SELF:
            parse_action_group_join("You");
            break;

        case RP_GROUP_LEAVE_SELF:
        case RP_GROUP_DISBAND:
            parse_action_group_leave("You");
            break;

        case RP_GROUP_JOIN_PLAYER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_join(name);
            break;

        case RP_GROUP_LEAVE_PLAYER:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            parse_action_group_leave(name);
            break;

        case RP_CHAT_GENERAL:
            re_strlcpy(name, matchstr, sizeof(name), rematch[1]);
            re_strlcpy(chat, matchstr, sizeof(chat), rematch[2]);

            parse_action_chat_general(name, chat);
            break;

        default:
            printf("Unknown RP ID %u\n", rp_id);
            break;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;
    int ii;

    if (!group_init(argc - 1, argv + 1))
    {
        printf("Unable to initialize the group.");
        return 0;
    }

    for (ii = 0; ii < sizeof(rp_aion) / sizeof(rp_aion[0]); ii++)
    {
        retval = regcomp(&rp_aion[ii].rp_comp, rp_aion[ii].rp_exp, REG_EXTENDED);
        if (retval != 0)
        {
            char errstr[64];

            regerror(retval, &rp_aion[ii].rp_comp, errstr, sizeof(errstr));
            printf("Error parsing regex: %s (%s)\n", rp_aion[ii].rp_exp, errstr);
            return 0;
        }
    }

    {
        FILE *f;
        regmatch_t rematch[16];
        char buf[1024];

        char *install_path = NULL;

        install_path = aion_get_install_path();
        if (install_path == NULL)
        {
            printf("Unable to find Aion install path.\n");
            return 1;
        }

        buf[0] = '\0';
        strlcat(buf, install_path, sizeof(buf));
        strlcat(buf, "\\Chat.log", sizeof(buf));

        printf("Opening chat file '%s'\n", buf);
        f = fopen(buf, "r");
        if (f == NULL)
        {
            printf("Error opening ifle\n");
            return 1;
        }

        // Seek to the end of file
        fseek(f, 0, SEEK_END);

        for (;;)
        {
            char *chopptr;

            if (fgets(buf, sizeof(buf), f) == NULL)
            {
                usleep(300);
                continue;
            }

            chopptr = buf + strlen(buf) - 1;
            /* Chop off new-line characters */

            while ((buf <= chopptr) && (*chopptr == '\n' || *chopptr == '\r') )
            {
                *chopptr-- = '\0';
            }

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

