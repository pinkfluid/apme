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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pcreposix.h>

#include "regeng.h"
#include "console.h"

/*
 * Copy a string from a matched regular expression
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

size_t re_strlen(regmatch_t rem)
{
    if ((rem.rm_so < 0) ||
        (rem.rm_eo < 0))
    {
        return 0;
    }

    return (rem.rm_eo - rem.rm_so);
}

/*
 * Initialize a regeng structure
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


bool re_parse(re_callback_t re_callback, struct regeng *re_array, char *str)
{
    struct regeng *reptr;
    regmatch_t rematch[RE_REMATCH_MAX];

    for (reptr = re_array; RE_REGENG_VALID(reptr); reptr++)
    {
        if (regexec(&reptr->re_comp, str, RE_REMATCH_MAX, rematch, 0) == 0)
        {
            re_callback(reptr->re_id, str, rematch, RE_REMATCH_MAX);
            return true;
        }
    }

    return true;
}

