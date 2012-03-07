/*
 * help.h - APme: Aion Automatic Abyss Point Tracker
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

#ifndef HELP_H_INCLUDED
#define HELP_H_INCLUDED

extern const char *help_chatlog_warning;
extern const char *help_chatlog_enabled;
extern const char *help_chatlog_enable_error;
extern const char *help_mainscreen;
extern const char *help_invfull_on;
extern const char *help_invfull_off;

extern void help_cmd(char *cmd, char *help, size_t help_sz);
extern void help_usage(char *usage, size_t usage_sz);

#endif /* HELP_H_INCLUDED */

