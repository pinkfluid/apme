#ifndef REGENG_H_INCLUDED
#define REGENG_H_INCLUDED

#include <pcreposix.h>

#define RE_MATCH(x)  (((x).rm_so != -1) && ((x).rm_eo != -1))

/* Marker for end of regeng */
#define RE_INVALID_ID       0xffffffff
#define RE_REMATCH_MAX      32

#define RE_REGENG_VALID(x)  (((x)->re_id  != RE_INVALID_ID) && \
                             ((x)->re_exp != NULL))

#define RE_REGENG_END       { .re_id = RE_INVALID_ID, .re_exp = NULL }

struct regeng
{
    uint32_t    re_id;
    regex_t     re_comp;
    char        *re_exp;
};

typedef void  re_callback_t(uint32_t re_id, const char *str, regmatch_t *rematch, size_t rematch_max);

extern bool   re_init(struct regeng *re_array);
extern bool   re_parse(re_callback_t re_callback, struct regeng *re_array, char *str);

extern void   re_strlcpy(char *outstr, const char *instr, size_t outsz, regmatch_t rem);
extern size_t re_strlen(regmatch_t rem);

#endif /* REGENG_H_INCLUDE */
