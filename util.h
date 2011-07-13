#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

extern size_t util_strlcpy(char *dst, const char *src, size_t dst_size);
extern size_t util_strlcat(char *dst, const char *src, size_t dst_size);

extern bool clipboard_set_text(char *text);
extern bool clipboard_get_text(char *text, size_t text_sz);

extern char* aion_get_install_path(void);

extern void util_chomp(char *str);

#endif // UTIL_H_INCLUDED
