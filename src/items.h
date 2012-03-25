/*
 * items.h - APme: Aion Automatic Abyss Point Tracker
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

#ifndef ITEM_H_INCLUDED
#define ITEM_H_INCLUDED

/**
 * @file 
 */

#include <stdint.h>

/**
 * @ingroup items
 *
 * @{
 */

/** Item structure  */
struct item
{
    uint32_t    item_id;        /**< The item ID                */
    char        *item_name;     /**< The item name              */
    uint32_t    item_ap;        /**< The AP value of this item  */
};

extern struct item* item_find(uint32_t itemid);
extern struct item* item_find_name(char *item_name);

/**
 * @}
 */

#endif // ITEM_H_INCLUDED
