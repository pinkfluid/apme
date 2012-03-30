/*
 * util.c - APme: Aion Automatic Abyss Point Tracker
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
 * Random utilities
 * 
 * @author Mitja Horvat <pinkfluid@gmail.com>
 */
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef SYS_WINDOWS
#include <windows.h>
#endif

#include "util.h"
#include "console.h"

/** 
 * @defgroup util Utilities
 * @brief Collection of verious utilities that do not have a place of their own, yet
 *
 * @note Only Windows specific functions are documented
 *
 * @{
 */

#ifdef SYS_WINDOWS
/**
 * Copy @p text to the clipboard
 *
 * @param[in]       text        Text to copy to the clipboard
 *
 * @retval          true        On success
 * @retval          false       If any of the Windows clipboard functions failed
 */
bool clipboard_set_text(char *text)
{
    bool retval;
    HGLOBAL hdst;
    char *dst;

    DWORD dst_sz = strlen(text) + sizeof('\0');

    /* Allocate and copy the string to the global memory */
    hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dst_sz);
    dst = (char *)GlobalLock(hdst);
    util_strlcpy(dst, text, dst_sz);
    GlobalUnlock(hdst);

    /* Set clipboard data */
    retval = OpenClipboard(NULL);
    if (!retval)
    {
        con_printf("Error opening clipboard\n");
        return false;
    }

    EmptyClipboard();

    retval = SetClipboardData(CF_TEXT, hdst);
    if (!retval)
    {
        con_printf("Error pasting to clipboard\n");
        return false;
    }

    CloseClipboard();
    return true;
}

/**
 * Store the content of the clipboard to @p text
 *
 * @param[out]      text        Buffer that will receive the buffer data
 * @param[in]       text_sz     Maximum size of @p text
 *
 * @retval          true        On success
 * @retval          false       On error
 */
bool clipboard_get_text(char *text, size_t text_sz)
{
    HGLOBAL hsrc;
    char *src;

    bool status = false;

    // Set clipboard data
    if (!OpenClipboard(NULL))
    {
        con_printf("Error opening clipboard\n");
        return false;
    }

    hsrc = GetClipboardData(CF_TEXT);
    if (hsrc == NULL)
    {
        // Nothing to retrieve
        goto error;
    }

    src = GlobalLock(hsrc);
    if (src == NULL)
    {
        con_printf("Error locking clipboard data\n");
        goto error;
    }

    util_strlcpy(text, src, text_sz);

    GlobalUnlock(hsrc);

    status = true;

error:
    CloseClipboard();
    return status;

}

/**
 * Store a registry key value to @p buf
 *
 * @param[in]       key     Registry key path
 * @param[in]       val     Registry key name
 * @param[out]      buf     Buffer that will receive the registry key value
 * @param[in]       buflen  Maximum size of buffer @p buf
 *
 * @retval          true    On success
 * @retval          false   If @p key is invalid
 * @retval          false   If @p val is invalid
 */
bool reg_read_key(char *key, char *val, void *buf, size_t buflen)
{
    LONG retval;
    HKEY key_handle;
    bool status = false;

    DWORD len = buflen;

    retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &key_handle);
    if (retval != ERROR_SUCCESS)
    {
        return false;
    }

    retval = RegQueryValueEx(key_handle, val, NULL, NULL, buf, &len);
    if (retval != ERROR_SUCCESS)
    {
        goto error;
    }

    status = true;

error:
    RegCloseKey(key_handle);
    return status;
}

#else /* Unix */

/**
 * @cond UTILS_UNIX
 */
bool clipboard_set_text(char *text)
{
    FILE *clipboard;

    /* On unix, just read from the clipboard.txt file :) */
    clipboard = fopen("clipboard.txt", "w+");
    if (clipboard != NULL)
    {
        fputs(text, clipboard);
        fputs("\n", clipboard);
        fclose(clipboard);
    }

    return true;
}

bool clipboard_get_text(char *text, size_t text_sz)
{
    FILE *clipboard;

    *text = '\0';

    /* On unix, just read from the clipboard.txt file :) */
    clipboard = fopen("clipboard.txt", "r");
    if (clipboard != NULL)
    {
        if (fgets(text, text_sz, clipboard) == NULL)
        {
            /* Ignore warning */
        }

        fclose(clipboard);
    }

    util_chomp(text);

    return true;
}

bool reg_read_key(char *key, char *val, void *buf, size_t buflen)
{
    (void)key;
    (void)val;
    (void)buf;
    (void)buflen;

    return true;
}

/**
 * @endcond
 */

#endif

/**
 * This is a safe string copy function, it never overflows. In the *BSD world it's know as strlcpy().
 *
 * Thansk to Ulrich Drepper, these two functions probably get the re-inventing-the-wheel-over-and-over-again award.
 *
 * @note If dst is too small to hold src, it is safely truncated.
 *
 * @param[out]      dst         Buffer that will receive the string from @p src
 * @param[in]       src         Input string
 * @param[in]       dst_size    Maximum size of @p dst; if a larger string is copied it is truncated and padded with '\\0'
 *
 * @return
 * Number of bytes stored to dst (not counting the ending null char)
 */
size_t util_strlcpy(char *dst, const char *src, size_t dst_size)
{
    size_t src_len = strlen(src);
    size_t dst_len;

    if (dst_size == 0) return 0;

    dst_len = ((src_len + 1) < dst_size) ? src_len : (dst_size - 1);

    memcpy(dst, src, dst_len);

    dst[dst_len] = '\0';

    return dst_len;
}

/**
 * Equivalent of the strlcat() function from the *BSD world. See @ref util_strlcpy() for the rant.
 *
 * @param[in,out]   dst         String that we're appending to
 * @param[in]       src         String that will be appended
 * @param[in]       dst_size    Maximum size of @p dst
 *
 * @return
 * Number of bytes stored to @p dst
 */
size_t util_strlcat(char *dst, const char *src, size_t dst_size)
{
    size_t src_len = strlen(src);
    size_t dst_len = strlen(dst);

    if (dst_size == 0) return 0;
    if (dst_len >= dst_size) assert(!"Invalid string passed to util_strlcat()");

    if (dst_size < (src_len + dst_len + 1)) src_len = dst_size - dst_len - 1;

    memcpy(dst + dst_len, src, src_len);

    dst[dst_len + src_len] = '\0';

    return dst_len + src_len;
}

/**
 * There's no strsep() on MinGW, so we have to implement our own
 * 
 * @note This function should be equivalent to a POSIX strsep()
 *
 * @param[in,out]       pinputstr   String to scan for @p delim
 * @param[in]           delim       Delimiters
 */
char* util_strsep(char **pinputstr, const char *delim)
{
    char *pstr;
    char *pend;

    if (*pinputstr == NULL) return NULL;

    pstr = pend = *pinputstr;

    pend += strcspn(pstr, delim);

    if (*pend == '\0')
    {
        *pinputstr = NULL;
    }
    else
    {
        *pend++ = '\0';
        *pinputstr = pend;
    }

    return pstr;
}

/**
 * This function this function removes new-lines characters
 * from the end of the string
 *
 * @param[in]       str     String to be chomped
 */
void util_chomp(char *str)
{
    char *pstr;

    pstr = str + strlen(str) - 1;
    while ((str <= pstr) && (*pstr == '\n' || *pstr == '\r') )
    {
        *pstr-- = '\0';
    }
}

/**
 * @}
 */
