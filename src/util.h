#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

/* XXX:  Move this to some sort of regex module */
#include <pcreposix.h>
#define RE_MATCH(x)  (((x).rm_so != -1) && ((x).rm_eo != -1))
extern void util_re_strlcpy(char *outstr, const char *instr, ssize_t outsz, regmatch_t rem);
extern size_t util_re_strlen(regmatch_t rem);
/* XXX:  Move this to some sort of regex module */


extern bool clipboard_set_text(char *text);
extern bool clipboard_get_text(char *text, size_t text_sz);

extern char* aion_get_install_path(void);

extern char* util_strsep(char **pinputstr, const char *delim);
extern size_t util_strlcpy(char *dst, const char *src, size_t dst_size);
extern size_t util_strlcat(char *dst, const char *src, size_t dst_size);
extern void util_chomp(char *str);

#endif // UTIL_H_INCLUDED
