#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "group.h"
#include "util.h"

struct group_member group[6];

bool group_init(int nmembers, char *names[])
{
    int ii;

    memset(group, 0, sizeof(group));
    /*
     *  A group consists of 6 persons maximum,
     *  However, one spot is reserved for the palyer
     */

    if (nmembers > 5)
    {
        nmembers = 5;
    }


    for (ii = 0; ii < nmembers; ii++)
    {
        /* Maximum of 5 group members */
        group[ii].gm_name   = names[ii];
        group[ii].gm_ap     = 0;
    }

    /* Finally add the last spot, the player */
    group[ii].gm_name       = "You";
    group[ii].gm_ap         = 0;

    return true;
}

void group_stats(void)
{
    int ii;

    printf("STATS: ");

    for (ii = 0; ii < sizeof(group) / sizeof(group[0]); ii++)
    {
        if (group[ii].gm_name == NULL) continue;

        printf("%s (AP: %u) | ", group[ii].gm_name, group[ii].gm_ap);
    }

    printf("\n");
}

void group_ap_update(char *name, uint32_t ap)
{
    int ii;

    for (ii = 0; ii < sizeof(group) / sizeof(group[0]); ii++)
    {
        if (group[ii].gm_name == NULL) continue;

        if (strcasecmp(group[ii].gm_name, name) == 0)
        {
            group[ii].gm_ap += ap;
        }
    }
}

void group_ap_eligible(void)
{
    char buf[48];
    char clip_buf[256];
    int ii;

    uint32_t lowest_ap = 0xffffffff;

    /* Find what is the lowest amount of AP */
    for (ii = 0; ii < sizeof(group) / sizeof(group[0]); ii++)
    {
        if (group[ii].gm_name == NULL) continue;

        if (group[ii].gm_ap < lowest_ap)
        {
            lowest_ap = group[ii].gm_ap;
        }
    }

    snprintf(clip_buf, sizeof(clip_buf), "ROLL: ");
    /* Now display the players */
    for (ii = 0; ii < sizeof(group) / sizeof(group[0]); ii++)
    {
        if (group[ii].gm_name == NULL) continue;
        if (group[ii].gm_ap != lowest_ap) continue;

        snprintf(buf, sizeof(buf), "%s | ", group[ii].gm_name);
        util_strlcat(clip_buf, buf, sizeof(clip_buf));
    }

    printf("%s\n", clip_buf);

    /* Copy this to the clipboard */
    clipboard_set_text(clip_buf);
}

