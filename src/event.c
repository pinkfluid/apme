/*
 * event.c - APme: Aion Automatic Abyss Point Tracker
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
 * Simple Events Library
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#include <stdlib.h>

#include "console.h"
#include "event.h"

/**
 * @defgroup event Event Subsystem
 * @brief 4 lines of code do not need extensive documentation.
 *
 * @{
 */

/** The event processing callback       */
static event_callback_t *event_process_cb = NULL;

/**
 * Register the event callback
 *
 * @param[in]       event_cb        The event processing function
 */ 
void event_register(event_callback_t *event_cb)
{
    event_process_cb = event_cb;
}

/**
 * Dispatch an event to the event processing function
 *
 * @param[in]       event           The event type
 */
void event_signal(enum event_type event)
{
    if (event_process_cb == NULL)
    {
        con_printf("Event fired, but no event callback defined!\n");
        return;
    }

    event_process_cb(event);
}

/**
 * @}
 */
