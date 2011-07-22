#ifndef REGENG_H_INCLUDED
#define REGENG_H_INCLUDED

#include <pcreposix.h>

#define RE_MATCH(x)  (((x).rm_so != -1) && ((x).rm_eo != -1))

struct regeng
{
    uint32_t    re_id;
    regex_t     re_comp;
    char        re_exp[256];
};

extern void   re_strlcpy(char *outstr, const char *instr, ssize_t outsz, regmatch_t rem);
extern size_t re_strlen(regmatch_t rem);

#endif /* REGENG_H_INCLUDE */
