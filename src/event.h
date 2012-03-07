/*
 * event.h - APme: Aion Automatic Abyss Point Tracker
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

#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

enum event_type
{
    EVENT_AION_GROUP_UPDATE     = 0,    /* Group has been updated       */
    EVENT_AION_AP_UPDATE        = 1,    /* AP value of a member update  */
    EVENT_AION_INVENTORY_FULL   = 2,    /* Somebody has inventory full  */
    EVENT_AION_LOOT_RIGHTS      = 3,    /* New loot rights calculated   */
};

typedef void event_callback_t(enum event_type ev);

extern void event_register(event_callback_t *event_cb);
extern void event_signal(enum event_type event);

#endif /* EVENT_H_INCLUDED */
