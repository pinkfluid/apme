#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

extern bool set_clipboard_text(char *text);
extern char* aion_get_install_path(void);

extern size_t util_strlcpy(char *dst, const char *src, size_t dst_size);
extern size_t util_strlcat(char *dst, const char *src, size_t dst_size);

#endif // UTIL_H_INCLUDED
