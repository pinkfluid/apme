/*
 * chatlog.h - APme: Aion Automatic Abyss Point Tracker
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

#ifndef CHATLOG_H_INCLUDED
#define CHATLOG_H_INCLUDED


#define CHATLOG_PREFIX_LEN      20              /* Length of the prefix for each
                                                    chatlog line, the timestamp */
#define CHATLOG_FILENAME        "Chat.log"
#define CHATLOG_CHAT_SZ         1024
#define CHATLOG_NAME_SZ         64
#define CHATLOG_ITEM_SZ         64

extern bool chatlog_init(void);
extern bool chatlog_poll(void);
extern bool chatlog_readfile(char *file);

#endif
