
#include <stdbool.h>
#include <stdio.h>

#include <windows.h>

#include "util.h"

bool set_clipboard_text(char *text)
{
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

    return true;
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

char *reg_aion_keys[] =
{
    "SOFTWARE\\NCsoft\\AionEU",     /* Aion Europe  */
    "SOFTWARE\\NCsoft\\Aion",       /* Aion US ?    */
};

char* aion_get_install_path(void)
{
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

    return NULL;
}


