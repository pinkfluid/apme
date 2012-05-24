/*
 * config.h - APme: Aion Automatic Abyss Point Tracker
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
 * @file config.h
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/** 
 * @addtogroup config 
 * @{
 */

/** Configuration file name */
#define CFG_APME_INI "apme.ini"

/** The application section string */
#define CFG_SEC_APP "apme"

/** Maximum CFG key size (section + ':' + name) */
#define CFG_KEYSZ   256

extern bool cfg_init(void);
extern bool cfg_load(void);
extern bool cfg_store(void);
extern bool cfg_set_string(char *section, char *name, char *value);
extern bool cfg_set_int(char *section, char *key, int value);
extern bool cfg_get_string(char *section, char *name, char *value, size_t valuesz);

extern void cfg_periodic(void);

/**
 * @}
 */

#endif /* CONFIG_H_INCLUDE */
