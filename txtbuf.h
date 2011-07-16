#ifndef TXTBUF_H_INCLUDED
#define TXTBUF_H_INCLUDED

#include <stdbool.h>

struct txtbuf
{
    size_t  tb_size;
    size_t  tb_head;
    size_t  tb_tail;
    char    *tb_text;
};

extern void tb_init(struct txtbuf *tb, char *txt, size_t txt_sz);
extern bool tb_put(struct txtbuf *tb, void *buf, size_t buf_sz);
extern void tb_strtrim(struct txtbuf *tb);
extern bool tb_strput(struct txtbuf *tb, char *str);
extern int tb_strnum(struct txtbuf *tb);
extern bool tb_strget(struct txtbuf *tb, int index, char *dst, size_t dst_sz);
extern bool tb_strlast(struct txtbuf *tb, int index, char *dst, size_t dst_sz);

#endif /* TXTBUF_H_INCLUDED */
