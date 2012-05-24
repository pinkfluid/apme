/*
 * version.h - APme: Aion Automatic Abyss Point Tracker
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
 * @file version.h
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */

#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

/** Major version number        */
#define APME_VERSION_MAJOR      0
/** Minor version number        */
#define APME_VERSION_MINOR      6
/** Revision number             */
#define APME_VERSION_REVISION   99
/** Version codename            */
#define APME_VERSION_NAME       "Development"

/** Convert parameter to string */
#define __APM_STR(x)            #x
/** Expand macro <I>x</I> and convert it to string */
#define APM_STR(x)              __APM_STR(x)

/** The concatenated version string in human redable form */
#define APME_VERSION_STRING  APM_STR(APME_VERSION_MAJOR) "." \
                             APM_STR(APME_VERSION_MINOR) "." \
                             APM_STR(APME_VERSION_REVISION) " "\
                             "(" APME_VERSION_NAME ")"

#endif /* VERSION_H_INCLUDED */
