/*
 * aion.h - APme: Aion Automatic Abyss Point Tracker
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

#ifndef AION_H_INCLUDED
#define AION_H_INCLUDED

#include "chatlog.h"

#define AION_NAME_DEFAULT   "You"
#define AION_CHAT_SZ        CHATLOG_CHAT_SZ
#define AION_NAME_SZ        CHATLOG_NAME_SZ

#define AION_SYSOVR_FILE    "system.ovr"
#define AION_SYSOVR_CHATLOG "g_chatlog"

/* This is the number characters that Aion allowts to be paste */
#define AION_CLIPBOARD_MAX 255

extern bool aion_init(void);
extern bool aion_clipboard_set(char *text);

extern bool aion_player_is_self(char *charname);
extern bool aion_group_join(char *charname);
extern bool aion_group_leave(char *charname);
extern void aion_group_disband(void);
extern void aion_group_loot(char *charname, uint32_t itemid);

extern void aion_apvalue_reset(void);
extern bool aion_group_apvalue_update(char *charname, uint32_t apval);
extern bool aion_group_apvalue_set(char *charname, uint32_t apval);
extern uint32_t aion_group_get_apvalue_lowest(void);

extern bool aion_invfull_set(char *charname, bool isfull);
extern bool aion_invfull_get(char *charname);
extern void aion_invfull_excl_set(bool enable);
extern bool aion_invfull_excl_get(void);
extern void aion_invfull_clear(void);

extern void aion_aplimit_set(uint32_t aplimit);
extern uint32_t aion_aplimit_get(void);

extern bool aion_player_chat_cache(char *charname, char *chat);
extern bool aion_player_chat_get(char *charname, int msgnum, char *dst, size_t dst_sz);
extern void aion_player_name_set(char *charname);

/* The group iterator */
struct aion_group_iter
{
    char                *agi_name;
    uint32_t            agi_apvalue;
    bool                agi_invfull;

    struct aion_player  *__agi_curplayer;
};

extern void aion_group_first(struct aion_group_iter *iter);
extern void aion_group_next(struct aion_group_iter *iter);
extern bool aion_group_end(struct aion_group_iter *iter);

#define LANG_ELYOS      1
#define LANG_ASMODIAN   2
#define LANG_KRALL      3
#define LANG_MAU        4
#define LANG_BALAUR     5

extern void aion_translate(char *txt, uint32_t language);
extern void aion_rtranslate(char *txt, uint32_t language);

extern bool aion_group_get_stats(char *stats, size_t stats_sz);
extern bool aion_group_get_aplootrights(char *stats, size_t stats_sz);

extern char* aion_default_install_path(void);
extern bool aion_chatlog_is_enabled(bool *isenabled);
extern bool aion_chatlog_enable(void);

#endif /* AION_H_INCLUDED */
