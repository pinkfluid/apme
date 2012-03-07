/*
 * util.h - APme: Aion Automatic Abyss Point Tracker
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

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

extern bool clipboard_set_text(char *text);
extern bool clipboard_get_text(char *text, size_t text_sz);

extern char* util_strsep(char **pinputstr, const char *delim);
extern size_t util_strlcpy(char *dst, const char *src, size_t dst_size);
extern size_t util_strlcat(char *dst, const char *src, size_t dst_size);
extern void util_chomp(char *str);

/* Registry stuff */
extern bool reg_read_key(char *key, char *val, void *buf, size_t buflen);

#endif // UTIL_H_INCLUDED
