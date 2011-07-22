#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pcreposix.h>

#include "regeng.h"

/*
 * Copy a string from a matched regular expression
 */
void re_strlcpy(char *outstr, const char *instr, ssize_t outsz, regmatch_t rem)
{
    ssize_t sz = rem.rm_eo - rem.rm_so;

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

