#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef SYS_WINDOWS
#include <windows.h>
#endif

#include "util.h"
#include "console.h"

#ifdef SYS_WINDOWS
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

#endif

/* 
 * Thansk to Ulrich Drepper, these two functions probably get the re-inventing-the-wheel-over-and-over-again award.
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

/*
 * There's no strsep() on MinGW, so we have to implement our own
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

void util_chomp(char *str)
{
    char *pstr;

    pstr = str + strlen(str) - 1;
    while ((str <= pstr) && (*pstr == '\n' || *pstr == '\r') )
    {
        *pstr-- = '\0';
    }
}

