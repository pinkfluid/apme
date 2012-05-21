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

/**
 * @file
 * Utilities
 * 
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdio.h>

/**
 * @ingroup util
 *
 * @{
 */

#ifdef SYS_WINDOWS
/** Max path on Windows */
#include <windows.h>
#define UTIL_MAX_PATH   MAX_PATH
#else
/** Max path on Unix */
#include <limits.h>
#define UTIL_MAX_PATH   PATH_MAX
#endif

extern bool clipboard_set_text(char *text);
extern bool clipboard_get_text(char *text, size_t text_sz);
extern bool sys_is_admin(bool *isadmin);
extern bool sys_runas_admin(char *path);
extern bool sys_self_exe(char *path, size_t pathsz);
extern bool sys_self_elevate(void);
extern bool sys_perm_grant(char *path);
extern FILE* sys_fopen_force(char *path, char *mode);
extern bool sys_appdata_path(char *path, size_t pathsz);

extern char* util_strsep(char **pinputstr, const char *delim);
extern size_t util_strlncat(char *dst, const char *src, size_t dst_size, size_t nchars);
extern size_t util_strlcpy(char *dst, const char *src, size_t dst_size);
extern size_t util_strlcat(char *dst, const char *src, size_t dst_size);
extern void util_strrep(char *out, size_t outsz, char *in,  char *findstr, char *replacestr);
extern void util_chomp(char *str);

/* Registry stuff */
extern bool reg_read_key(char *key, char *val, void *buf, size_t buflen);

/* Codepage stuff */
void util_cp1252_to_utf8(char *utf8, ssize_t utf8_sz, char *cp1252);

/**
 * @}
 */
#endif // UTIL_H_INCLUDED
