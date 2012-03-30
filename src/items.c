/*
 * items.c - APme: Aion Automatic Abyss Point Tracker
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
 * Simple Item Database
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include "items.h"

/**
 * @defgroup items Simple Item Database
 *
 * @brief Currnetly it contains only the IDs for the AP relics
 *
 * @{
 */

/** Items database */
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
        .item_name  = "Greater Ancient Seal",
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
        .item_name  = "Greater Ancient Goblet",
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
};


/**
 * Find item with ID @p itemid in the database
 *
 * @param[in]       itemid      The item ID
 *
 * @return
 * Returns NULL on error or a pointer to an item in the database
 */
struct item* item_find(uint32_t itemid)
{
    size_t ii;

    for (ii = 0; ii < sizeof(itemdb)/sizeof(itemdb[0]); ii++)
    {
        if (itemdb[ii].item_id == itemid)
        {
            return &itemdb[ii];
        }
    }

    return NULL;
}

/**
 * Lookup an item by the item name
 *
 * @param[in]       item_name   The item name
 *
 * @return
 * Returns NULL on error or a pointer to an item in the database
 */
struct item* item_find_name(char *item_name)
{
    size_t ii;

    for (ii = 0; ii < sizeof(itemdb)/sizeof(itemdb[0]); ii++)
    {
        if (strcasecmp(itemdb[ii].item_name, item_name) == 0)
        {
            return &itemdb[ii];
        }
    }

    return NULL;
}

/**
 * @}
 */
