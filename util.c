
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <regex.h>

#ifdef SYS_WINDOWS
#include <windows.h>
#endif

#include "util.h"

/*
 * Copy a string from a matched regular expression
 */
void util_re_strlcpy(char *outstr, const char *instr, ssize_t outsz, regmatch_t rem)
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


    strncpy(outstr, instr + rem.rm_so, sz);

    /* Terminate it with NULL */
    outstr[sz] = '\0';
}


bool clipboard_set_text(char *text)
{
#ifdef SYS_WINDOWS
    bool retval;
    DWORD len = strlen(text);
    HGLOBAL hdst;
    LPWSTR dst;

    // Allocate string for cwd
    hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, len + 1);

    dst = (LPWSTR)GlobalLock(hdst);
    memcpy(dst,  text, len + 1);
    GlobalUnlock(hdst);

    // Set clipboard data
    retval = OpenClipboard(NULL);
    if (!retval)
    {
        printf("Error opening clipboard\n");
        return false;
    }

    EmptyClipboard();

    retval = SetClipboardData(CF_TEXT, hdst);
    if (!retval)
    {
        printf("Error pasting to clipboard\n");
        return false;
    }

    CloseClipboard();
#endif

    return true;
}

bool clipboard_get_text(char *text, size_t text_sz)
{
#ifdef SYS_WINDOWS
    HGLOBAL hsrc;
    char *src;

    bool status = false;

    // Set clipboard data
    if (!OpenClipboard(NULL))
    {
        printf("Error opening clipboard\n");
        return false;
    }

    hsrc = GetClipboardData(CF_TEXT);
    if (hsrc == NULL)
    {
        printf("Nothing in clipboard\n");
        // Nothing to retrieve
        goto error;
    }

    src = GlobalLock(hsrc);
    if (src == NULL)
    {
        printf("Error locking clipboard data\n");
        goto error;
    }

    util_strlcpy(text, src, text_sz);

    GlobalUnlock(hsrc);

    status = true;

error:
    CloseClipboard();
    return status;
#else
    return true;
#endif
}

bool reg_read_key(char *key, char *val, void *buf, size_t buflen)
{
#ifdef SYS_WINDOWS
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
#else
    return true;
#endif
}

char *reg_aion_keys[] =
{
    "SOFTWARE\\NCsoft\\AionEU",     /* Aion Europe  */
    "SOFTWARE\\NCsoft\\Aion",       /* Aion US ?    */
};

char* aion_get_install_path(void)
{
#ifdef SYS_WINDOWS
    static char aion_install_path[1024];
    int ii;
    bool retval;

    for (ii = 0; ii < sizeof(reg_aion_keys) / sizeof(reg_aion_keys[0]); ii++)
    {
        retval = reg_read_key(reg_aion_keys[ii],
                              "InstallPath",
                              aion_install_path,
                              sizeof(aion_install_path));
        if (retval)
        {
            return aion_install_path;
        }
    }
#endif

    return NULL;
}

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

void util_chomp(char *str)
{
    char *pstr;

    pstr = str + strlen(str) - 1;
    while ((str <= pstr) && (*pstr == '\n' || *pstr == '\r') )
    {
        *pstr-- = '\0';
    }
}
