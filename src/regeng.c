/*
 * regeng.c - APme: Aion Automatic Abyss Point Tracker
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
 * The Regular Expression Engine
 *
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pcreposix.h>

#include "regeng.h"
#include "console.h"

/**
 * @defgroup regeng The Regular Expression Engine
 *
 * @brief The regular expression engine
 *
 * This module was written first by using regular POSIX-regex. Since
 * there's no POSIX regex support in MinGW, it was necessary to use
 * an external regex library -- PCRE.
 *
 * @note This is the only module that heavily relies on 3rd party libraries.
 *
 * @{
 */

/**
 * Extract a sub-string that was matched by the regular expression
 * in @p rem
 *
 * @note This function will properly truncate the string if the output string
 * is too small and place a '\\0' at the end without causing any overflows.
 *
 * @param[out]      outstr      Sub-string pointer
 * @param[in]       instr       Full-string on which regex was performed
 * @param[in]       outsz       Maximum size of the output sub-string
 * @param[in]       rem         The regex match structure
 */
void re_strlcpy(char *outstr, const char *instr, size_t outsz, regmatch_t rem)
{
    size_t sz = rem.rm_eo - rem.rm_so;

    /* We cannot copy 0 bytes */
    if ((outsz == 0) || (outstr == NULL))
    {
        /* I know this message makes 0 sense, but that makes it unique too */
        assert(!"Unable to copy 0 bytes to NULL");
    }

    /* Sanity checks */
    if ((rem.rm_so < 0) || 
        (rem.rm_eo < 0) ||
        (sz <= 0))
    {
        *outstr = '\0';
    }

    /* Check if the out string has enough space + the terminating NULL character */
    if ((sz + 1) > outsz)
    {
        /* Cap the size */
        sz = outsz - 1;
    }

    memcpy(outstr, instr + rem.rm_so, sz);

    /* Terminate it with NULL */
    outstr[sz] = '\0';
}

/**
 * Returns the length of the matched regex 
 *
 * @param[in]       rem     Matched regex
 *
 * @return
 * Number of characters that was matched in @p rem
 */
size_t re_strlen(regmatch_t rem)
{
    if ((rem.rm_so < 0) ||
        (rem.rm_eo < 0))
    {
        return 0;
    }

    return (rem.rm_eo - rem.rm_so);
}

/**
 * Initialize the regular expression engine 
 *
 * Scan the regex array and compile the regular expressions in the @p re_exp field
 *
 * @param[in]       re_array        Array of regular expression structures
 *
 * @return
 * true on success, or false if any of the regular expressions failed to
 * initialize.
 */
bool re_init(struct regeng *re_array)
{
    char errstr[64];
    struct regeng *reptr;

    for (reptr = re_array; RE_REGENG_VALID(reptr); reptr++)
    {
        int retval;

        retval = regcomp(&reptr->re_comp, reptr->re_exp, REG_EXTENDED);
        if (retval == 0) continue;

        regerror(retval, &reptr->re_comp, errstr, sizeof(errstr));
        con_printf("Error parsing regex: %s (%s)\n", reptr->re_exp, errstr);

        return false;
    }

    return true;
}

/**
 * This is the main loop of the regular expression engine
 *
 * @note @p re_array must have been initialized with re_init() before
 * calling this function
 *
 * @param[in]       re_array        Initialized regular expression array
 * @param[in]       re_callback     Callback that will be called for processing any matches
 * @param[in]       str             String to match
 *
 * @return
 * Currently returns always true
 */
bool re_parse(re_callback_t re_callback, struct regeng *re_array, char *str)
{
    struct regeng *reptr;
    regmatch_t rematch[RE_REMATCH_MAX];

    for (reptr = re_array; RE_REGENG_VALID(reptr); reptr++)
    {
        if (regexec(&reptr->re_comp, str, RE_REMATCH_MAX, rematch, 0) == 0)
        {
            con_printf("RE: '%s' matched by '%s', id:%d\n", str, reptr->re_exp, reptr->re_id);
            re_callback(reptr->re_id, str, rematch, RE_REMATCH_MAX);
            return true;
        }
    }

    return true;
}


/**
 * @}
 */
