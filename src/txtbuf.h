/*
 * txtbuf.h - APme: Aion Automatic Abyss Point Tracker
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
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */ 
#ifndef TXTBUF_H_INCLUDED
#define TXTBUF_H_INCLUDED

#include <stdbool.h>

/**
 * @ingroup txtbuf
 *
 * @{
 */

/** Text buffer structure */
struct txtbuf
{
    size_t  tb_size;        /**< Text buffer fixed size         */
    size_t  tb_head;        /**< Text buffer head pointer       */
    size_t  tb_tail;        /**< Text buffer tail pointer       */
    char    *tb_text;       /**< Text buffer data               */
};

extern void tb_init(struct txtbuf *tb, char *txt, size_t txt_sz);
extern bool tb_put(struct txtbuf *tb, void *buf, size_t buf_sz);
extern void tb_strtrim(struct txtbuf *tb);
extern bool tb_strput(struct txtbuf *tb, char *str);
extern int tb_strnum(struct txtbuf *tb);
extern bool tb_strget(struct txtbuf *tb, int index, char *dst, size_t dst_sz);
extern bool tb_strlast(struct txtbuf *tb, int index, char *dst, size_t dst_sz);

/**
 * @}
 */
#endif /* TXTBUF_H_INCLUDED */
