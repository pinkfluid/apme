#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "items.h"

struct item itemdb[] =
{
    /* Icons */
    {
        .item_id    = 186000066,
        .item_name  = "Lesser Ancient Icon",
        .item_ap    = 300,
    },
    {
        .item_id    = 186000065,
        .item_name  = "Ancient Icon",
        .item_ap    = 600,
    },
    {

        .item_id    = 186000064,
        .item_name  = "Greater Ancient Icon",
        .item_ap    = 900,
    },
    {
        .item_id    = 186000063,
        .item_name  = "Major Ancient Icon",
        .item_ap    = 1200,
    },
    /* Seals */
    {
        .item_id    = 186000062,
        .item_name  = "Lesser Ancient Seal",
        .item_ap    = 600,
    },
    {
        .item_id    = 186000061,
        .item_name  = "Ancient Seal",
        .item_ap    = 1200,
    },
    {
        .item_id    = 186000060,
        .item_name  = "Greter Ancient Seal",
        .item_ap    = 1800,
    },
    {
        .item_id    = 186000059,
        .item_name  = "Major Ancient Seal",
        .item_ap    = 2400,
    },
    /* Goblets */
    {
        .item_id    = 186000058,
        .item_name  = "Lesser Ancient Goblet",
        .item_ap    = 1200,
    },
    {
        .item_id    = 186000057,
        .item_name  = "Ancient Goblet",
        .item_ap    = 2400,
    },
    {
        .item_id    = 186000056,
        .item_name  = "Greter Ancient Goblet",
        .item_ap    = 3600,
    },
    {
        .item_id    = 186000055,
        .item_name  = "Major Ancient Goblet",
        .item_ap    = 4800,
    },
    /* Crowns */
    {
        .item_id    = 186000054,
        .item_name  = "Lesser Ancient Crown",
        .item_ap    = 2400,
    },
    {
        .item_id    = 186000053,
        .item_name  = "Ancient Crown",
        .item_ap    = 4800,
    },
    {
        .item_id    = 186000052,
        .item_name  = "Greater Ancient Crown",
        .item_ap    = 7200,
    },
    {
        .item_id    = 186000051,
        .item_name  = "Major Ancient Crown",
        .item_ap    = 9600,
    },
#if 0
    /* TEST :) */
    {
        .item_id    = 152000202,
        .item_name  = "Titanium Ore",
        .item_ap    = 1,
    },
    {
        .item_id    =  152000203,
        .item_name  = "Greater Titanium Ore",
        .item_ap    = 10,
    }
#endif
};


struct item* item_find(uint32_t itemid)
{
    int ii;

    for (ii = 0; ii < sizeof(itemdb)/sizeof(itemdb[0]); ii++)
    {
        if (itemdb[ii].item_id == itemid)
        {
            return &itemdb[ii];
        }
    }

    return NULL;
}

struct item* item_find_name(char *item_name)
{
    int ii;

    for (ii = 0; ii < sizeof(itemdb)/sizeof(itemdb[0]); ii++)
    {
        if (strcasecmp(itemdb[ii].item_name, item_name) == 0)
        {
            return &itemdb[ii];
        }
    }

    return NULL;
}
