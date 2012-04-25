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

/** 
 * @addtogroup aion
 * @{
 * 
 * @file aion.h Aion Subsystem 
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */

#include "chatlog.h"

/**
 * Default name for the player's character
 */
#define AION_NAME_DEFAULT   "You"
/**
 * Maximum chat line size in bytes
 */
#define AION_CHAT_SZ        CHATLOG_CHAT_SZ
/**
 * Maximum character name size
 */
#define AION_NAME_SZ        CHATLOG_NAME_SZ

/** Name of the system.ovr file */
#define AION_SYSOVR_FILE    "system.ovr"

/** Name of the chatlog variable in the system.ovr file */
#define AION_SYSOVR_CHATLOG "g_chatlog"

/** This is the number characters that Aion allowts to be paste */
#define AION_CLIPBOARD_MAX 255

/** Short aploot format */
#define AION_APLOOT_FORMAT_SHORT    "/ROLL (@ap AP):/ @name/// | INV FULL:/ @name/"
/** Medium aploot format, show pass list, without individual AP value */
#define AION_APLOOT_FORMAT_MED      "/ROLL (@ap AP):/ @name/ | PASS/ @name/ | INV FULL:/ @name/"
/** Long aploot format, show pass list, with individual AP format */
#define AION_APLOOT_FORMAT_LONG     "/ROLL:/ @name[@ap]/ | PASS:/ @name[@ap]/ | INV FULL:/ @name/"
/** Default format */
#define AION_APLOOT_FORMAT_DEFAULT  AION_APLOOT_FORMAT_SHORT

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
extern uint32_t aion_group_apvalue_lowest(void);

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

/**
 * The group iterator structure
 *
 * @see aion_group_first()
 * @see aion_group_end()
 * @see aion_group_next()
 */
struct aion_group_iter
{
    char                *agi_name;          /**< Player name                */
    uint32_t            agi_apvalue;        /**< Accumulated abyss points   */
    bool                agi_invfull;        /**< Inventory full flag        */

    struct aion_player  *__agi_curplayer;   /**< Internal, do not use       */
};

extern void aion_group_first(struct aion_group_iter *iter);
extern void aion_group_next(struct aion_group_iter *iter);
extern bool aion_group_end(struct aion_group_iter *iter);

/** Elyos language ID, see @ref aion_translate()    */
#define LANG_ELYOS      1
/** Asmodian language ID, see @ref aion_translate() */
#define LANG_ASMODIAN   2
/** Not used                                        */
#define LANG_KRALL      3
/** Not used                                        */
#define LANG_MAU        4
/** Not used, but it would be funny it was          */
#define LANG_BALAUR     5

extern void aion_translate(char *txt, uint32_t language);
extern void aion_rtranslate(char *txt, uint32_t language);

extern bool aion_aploot_stats(char *stats, size_t stats_sz);
extern bool aion_aploot_rights(char *stats, size_t stats_sz);
extern bool aion_aploot_fmt_parse(char *fmt);
extern bool aion_aploot_fmt_set(char *fmt);

extern char* aion_default_install_path(void);
extern bool aion_chatlog_is_enabled(bool *isenabled);
extern bool aion_chatlog_enable(void);

/**
 * @}
 */
#endif /* AION_H_INCLUDED */

