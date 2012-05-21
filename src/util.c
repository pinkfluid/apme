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
#ifdef SYS_WINDOWS
#include <windows.h>
#include <winnt.h>
#include <aclapi.h>
#include <shlobj.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "console.h"
#include "event.h"

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

/**
 * Check whether we're running with elevated privileges
 * (as Administrator)
 *
 * @param[out]  isadmin @p true if we're running as admin, @p false otherwise
 *
 * @note This function only works if compiled under MinGW
 *
 * @retval      true    On success
 * @retval      false   On error
 */
bool sys_is_admin(bool *isadmin)
{
#ifdef OS_CYGWIN
    /*
     * Under Cygwin the elevation stuff does not work, and the binary crashes
     * because it cannot find the .DLL anyway
     */
    (void)isadmin;
    return false;
#else
    HANDLE proc_token;
    BOOL ok;

    (void)isadmin;

    /* Open the current process' token */
    ok = OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &proc_token);
    if (!ok)
    {
        con_printf("OpenProcessToken() failed\n");
        return false;
    }

    TOKEN_ELEVATION_TYPE ele_type;
    DWORD ele_size;

    ok = GetTokenInformation(proc_token,
                             TokenElevationType,
                             &ele_type,
                             sizeof(ele_type),
                             &ele_size);
    if (!ok)
    {
        con_printf("GetTokenInformation() failed\n");
        return false;
    }

    switch (ele_type)
    {
        case TokenElevationTypeDefault:
            /*
             * TokenElevationTypeDefault is apparently returned when UAC is disabled; does this
             * also mean we have admin rights?
             */
        case TokenElevationTypeFull:
        default:
            *isadmin = true;
            break;

        case TokenElevationTypeLimited:
            *isadmin = false;
    }

    con_printf("sys_is_admin() says admin = %s!\n", *isadmin ? "TRUE" : "FALSE");

    return true;
#endif
}

/**
 * Execute process @p path with admin privileges
 *
 * @note This function will trigger the UAC dialog
 *
 * @param[in]   path    Path to the executable
 *
 * @retval      true    If process was executed successfully
 * @retval      false   On error, or if the user declined the UAC
 */
bool sys_runas_admin(char *path)
{
    BOOL success;
    SHELLEXECUTEINFO shexe_info;

    bool isadmin = false;

    printf("Trying to execute '%s' as admin!\n", path);
    /* Check if we're already admin */
    if (!sys_is_admin(&isadmin))
    {
        con_printf("Unable to determine if we're running elevated.\n");
        return false;
    }

    if (isadmin)
    {
        con_printf("Unable to execute %s as we're already elevated.\n", path);
        return false;
    }

    /* Do the undocumented "runas" trick of ShellExecute() */
    memset(&shexe_info, 0, sizeof(shexe_info));
    shexe_info.cbSize = sizeof(shexe_info);

    shexe_info.lpVerb = "runas";
    shexe_info.lpFile = path;
    shexe_info.nShow = SW_MAXIMIZE;

    success = ShellExecuteEx(&shexe_info);
    return (success == TRUE);
}

/**
 * Find out the path to the executable of the current process
 *
 * @param[out]  path        The path to our executable
 * @param[in]   pathsz      Maximum size of @p path in bytes
 *
 * @retval      true        On success
 * @retval      false       On error
 */
bool sys_self_exe(char *path, size_t pathsz)
{
    char exepath[MAX_PATH];
    DWORD retval;

    retval =  GetModuleFileName(NULL, exepath, sizeof(exepath));
    if (retval == 0)
    {
        con_printf("GetModuleFileName() failed.\n");
        return false;
    }

    con_printf("Our executable path is %s\n", exepath);
    util_strlcpy(path, exepath, pathsz);

    return true;
}

/**
 * Respawn the current process with elevated privileges
 *
 * @note This function will trigger the UAC dialog
 *
 * @retval      true        On success
 * @retval      false       On error or user declined UAC
 *
 * @see sys_self_exe()
 * @see sys_runas_admin()
 */
bool sys_self_elevate(void)
{
    char selfexe[MAX_PATH];

    /* Retrieve the path of the current executable */
    if (!sys_self_exe(selfexe, sizeof(selfexe)))
    {
        return false;
    }

    /* ... and run it as admin */
    if (!sys_runas_admin(selfexe))
    {
        con_printf("User might have declined the UAC.\n");
        return false;
    }

    return true;
}

/**
 * Grant full control permissions to everyone for the file in @p path
 *
 * @param[in]   path        Path to the file 
 *
 * @retval      true        On success
 * @retval      false       On error
 *
 * @note This code is based upon <a href="http://stackoverflow.com/questions/910528/how-to-change-the-acls-from-c">
 * this Stack Overflow post</a>
 */
bool sys_perm_grant(char *path)
{
    DWORD status;
    BOOL ok;

    bool retval = false;
    PACL acl = NULL;
    PSID psid_other = NULL;
    PSECURITY_DESCRIPTOR sec_desc = NULL;

    con_printf("Granting read access to everyone to the following file: %s\n", path);

    /* Chant the windows security magic */
    SID_IDENTIFIER_AUTHORITY sid_world = { SECURITY_WORLD_SID_AUTHORITY };

    ok = AllocateAndInitializeSid(&sid_world, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &psid_other);
    if (!ok)
    {
        con_printf("AllocateAndinitializeSid() failed\n");
        goto error;
    }

    /* Om om om om security magic om om om */
    EXPLICIT_ACCESS expl_acc[1];

    memset(&expl_acc, 0, sizeof(expl_acc));
    expl_acc[0].grfAccessPermissions    = 0xFFFFFFFF;
    expl_acc[0].grfAccessMode           = GRANT_ACCESS;
    expl_acc[0].grfInheritance          = NO_INHERITANCE;
    expl_acc[0].Trustee.TrusteeForm     = TRUSTEE_IS_SID;
    expl_acc[0].Trustee.TrusteeType     = TRUSTEE_IS_WELL_KNOWN_GROUP;
    expl_acc[0].Trustee.ptstrName       = psid_other;

    /* Om om om black magic om om om */
    status = SetEntriesInAcl(1, expl_acc, NULL, &acl);
    if (status != ERROR_SUCCESS)
    {
        con_printf("Error initializing ACLs with SetEntriesInAcl()\n");
    }

    /* Om om om best security ever om om om */
    sec_desc = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (sec_desc == NULL)
    {
        con_printf("LocalAlloc() failed when trying to allocate a security descriptor\n");
        goto error;
    }

    ok = InitializeSecurityDescriptor(sec_desc, SECURITY_DESCRIPTOR_REVISION);
    if (!ok)
    {
        con_printf("InitializeSecurityDescriptor() didn't want to cooperate\n");
        goto error;
    }

    ok = SetSecurityDescriptorDacl(sec_desc, TRUE, acl, FALSE);
    if (!ok)
    {
        con_printf("Unable set ACL on the security descriptor\n");
        goto error;
    }

    ok = SetFileSecurity(path, DACL_SECURITY_INFORMATION, sec_desc);
    if (!ok)
    {
        con_printf("Unable to set security on %s\n", path);
    }

    retval = true;

error:
    if (psid_other != NULL)
    {
        FreeSid(psid_other);
    }

    if (acl != NULL)
    {
        LocalFree(acl);
    }

    if (sec_desc != NULL)
    {
        LocalFree(sec_desc);
    }

    return retval;
}

/**
 * Work hard to open the file in @p path
 *
 * If we don't have admin rights, and we get a permission error,
 * request elevation.
 *
 * If we're admin, this function will fix the permissions of the file in @p path
 * by granting full control access to everyone.
 *
 *
 * @param[in]       path    Full path to file
 * @param[in]       mode    fopen() mode
 *
 * @return
 * This functions returns a FILE descriptor or NULL on error
 */
FILE *sys_fopen_force(char *path, char *mode)
{
    FILE *f;

    bool isadmin;

    /* Check if we have admin rights */
    if (!sys_is_admin(&isadmin))
    {
        isadmin = false; 
    }

    con_printf("sys_force_open(): %s (admin = %s)\n", path, isadmin ? "TRUE" : "FALSE");

    /* We do, fix the permissions of the file before opening it */
    if (isadmin)
    {
        con_printf("Fixing permissions on %s\n", path);
        if (!sys_perm_grant(path))
        {
            /* This might happen if the file does not exists, so log it */
            con_printf("Error fixing permissions on %s\n", path);
        }
    }

    /*  If we get a permission denied, fire the "elevation request event */
    f = fopen(path, mode);
    if (f != NULL)
    {
        /* Now return the file descriptor */
        return f;
    }

    /*
     * If it is an "access denied" type of error, try to elevate our privileges unless
     * we're already admin
     */
    if ((errno == EPERM) || (errno == EACCES))
    {
        if (isadmin)
        {
            con_printf("We're already admin and still cannot access the file.\n");
        }
        else
        {
            con_printf("Access denied when opening %s, requesting elevation\n", path);
            /* EVENT_SYS_ELEVATE_CHATLOG is usually a point of no return */
            event_signal(EVENT_SYS_ELEVATE_REQUEST);
        }
    }

    return NULL;
}

/**
 * Return the local APPDATA directory for APme.
 *
 * Create the folder if it does not exist.
 *
 * @param[out]      path    Path to the APPDATA directory
 * @param[in]       pathsz  Maximum number of characters that @p path can hold
 *
 * @retval          true    On success
 * @Retval          false   On error
 */
bool sys_appdata_path(char *path, size_t pathsz)
{
    char appdata_path[MAX_PATH];
    char *pstr;
    HRESULT hres;
    
    /* Retrieve the APP data path */
    hres = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appdata_path);
    if (!SUCCEEDED(hres))
    {
        con_printf("Error retrieving the LOCAL APPDATA path\n");
        return false;
    }

    /* Replace '\' with '/' */
    pstr = appdata_path;
    while (*pstr != '\0')
    {
        if (*pstr == '\\') *pstr = '/';
    }

    util_strlcat(appdata_path, "/APme", sizeof(appdata_path));

    con_printf("APPDATA directory: %s\n", appdata_path);

    /* Check if the directory exists, otherwise create it */
    if (!CreateDirectory(appdata_path, NULL))
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            con_printf("Create directory on '%s' failed\n", appdata_path);
            return false;
        }
    }

    util_strlcpy(path, appdata_path, pathsz);

    return true;
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

bool sys_is_admin(bool *isadmin)
{
    (void)isadmin;
    return false;
}

bool sys_runas_admin(char *path)
{
    (void)path;
    return false;
}

bool sys_self_exe(char *path, size_t pathsz)
{
    (void)path;
    (void)pathsz;
    return false;
}

bool sys_self_elevate(void)
{
    return true;
}

bool sys_perm_grant(char *path)
{
    (void)path;
    return false;
}

FILE *sys_fopen_force(char *path, char *mode)
{
    return fopen(path, mode);
}

bool sys_appdata_path(char *path, size_t pathsz)
{
    util_strlcpy(path, "./", pathsz);
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
 * Equivalent of the strncat() except it copies the string safely while
 * padding it with '@\0'
 *
 * @param[in,out]   dst         String that we're appending to
 * @param[in]       src         String that will be appended
 * @param[in]       dst_size    Maximum size of @p dst
 * @param[in]       nchars      Maximum number of characters to copy from @p src
 *
 * @return
 * Number of bytes stored to @p dst
 */
size_t util_strlncat(char *dst, const char *src, size_t dst_size, size_t nchars)
{
    size_t src_len = strlen(src);
    size_t dst_len = strlen(dst);

    if (dst_size == 0) return 0;
    if (dst_len >= dst_size) assert(!"Invalid string passed to util_strlcat()");

    if (src_len > nchars)
    {
        src_len = nchars;
    }

    if (dst_size < (src_len + dst_len + 1)) src_len = dst_size - dst_len - 1;

    memcpy(dst + dst_len, src, src_len);

    dst[dst_len + src_len] = '\0';

    return dst_len + src_len;
}

/**
 * Equivalent of the strlcat() function from the *BSD world. See @ref util_strlcpy() for the rant.
 *
 * @note This function is just a wrapper for util_strlncat()
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
    return util_strlncat(dst, src, dst_size, strlen(src));
}

/**
 * Stirng replace: Find all occurences of string @p findstr in string @p in and replace
 * them with the string @p replacestr. Store the result to @p out
 *
 * @param[out]      out         Output buffer where the result will be stored
 * @param[in]       outsz       Maximum size of the output buffer
 * @param[in]       in          Input string, this string will be searched for @p findstr
 * @param[in]       findstr     String to find
 * @param[in]       replacestr  String to replace @p findstr with
 */
void util_strrep(char *out, size_t outsz, char *in,  char *findstr, char *replacestr)
{
    char *pstr;
    char *pin;

    size_t rlen = strlen(replacestr);
    size_t flen = strlen(findstr);

    *out = '\0';
    pstr = pin = in;

    while ((pstr = strstr(pin, findstr)) != NULL)
    {
        /* Add everything before the matched pattern */
        util_strlncat(out, pin, outsz, pstr - pin);
        /* Add the replace pattern */
        util_strlncat(out, replacestr, outsz, rlen);
        /* Move the head forward */
        pin = pstr + flen;
    }

    /* Add the reminder of the string */
    util_strlncat(out, pin, outsz, strlen(pin));
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
 * and blanks from the end of the string
 *
 * @param[in]       str     String to be chomped
 */
void util_chomp(char *str)
{
    char *pstr;

    char blanks[] = " \r\n";

    pstr = str + strlen(str) - 1;
    while ((str <= pstr) && (strchr(blanks, *pstr) != NULL))
    {
        *pstr-- = '\0';
    }
}

/**
 * Convert a string from the CP-1252(aka Windows-1252, Latin1) codeset
 * to UTF8, yay!
 *
 * @param[in]       cp1252      Input string in the CP-1252 encoding
 * @param[out]      utf8        Output UTF8 string
 * @param[out]      utf8_sz     Maximum size of the UTF8 string
 */
void util_cp1252_to_utf8(char *utf8, ssize_t utf8_sz, char *cp1252)
{
    char *putf8 = utf8;
    char *pcp1252;

    for (pcp1252 = cp1252; *pcp1252 != 0x00; pcp1252++)
    {
        if (*(unsigned char *)pcp1252 < 0x80)
        {
            if ((putf8 - utf8) + 1 >= utf8_sz) break;

            *putf8++ = *pcp1252;
        }
        else if (*(unsigned char *)pcp1252 < 0xC0)
        {
            if ((putf8 - utf8) + 2 >= utf8_sz) break;
            *putf8++ = '\xC2';
            *putf8++ = *pcp1252;
        }
        else
        {
            if ((putf8 - utf8) + 2 >= utf8_sz) break;
            *putf8++ = '\xC3';
            *putf8++ = *pcp1252 - 64;
        }
    }

    *putf8 = '\0';
}

/**
 * @}
 */
