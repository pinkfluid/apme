#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "txtbuf.h"

void tb_init(struct txtbuf *tb, char *txt, size_t txt_sz)
{
    tb->tb_head = 0;
    tb->tb_tail = 0;
    tb->tb_text = txt;
    tb->tb_size = txt_sz;
};

inline size_t tb_len(struct txtbuf *tb)
{
    return tb->tb_tail - tb->tb_head;
}

inline size_t tb_free(struct txtbuf *tb)
{
    return tb->tb_size - tb_len(tb);
}

bool tb_put(struct txtbuf *tb, void *buf, size_t buf_sz)
{
    char *cbuf = (char *)buf;
    size_t tb_off;

    if (buf_sz > tb_free(tb))
    {
        return false;
    }

    while (buf_sz > 0)
    {
        size_t copy_sz;

        tb_off = tb->tb_tail % tb->tb_size;

        if ((tb_off + buf_sz) < tb->tb_size)
        {
            copy_sz = buf_sz;
        }
        else 
        {
            copy_sz = tb->tb_size - tb_off;
        }

        memcpy(tb->tb_text + tb_off, cbuf, copy_sz);
        
        /* Advance pointers by copy_sz positions */
        tb->tb_tail += copy_sz;
        cbuf        += copy_sz;      /* Move destination buffer forward              */
        buf_sz      -= copy_sz;      /* Decrease the number of bytes left to copy    */
    }

    return true; 
}

/*
 * Move the head forward to the beginning of the next string
 */
void tb_strtrim(struct txtbuf *tb)
{
    size_t ii;

    for (ii = tb->tb_head; ii < tb->tb_tail; ii++)
    {
        if (tb->tb_text[ii % tb->tb_size] == '\0') break;
    }

    if (ii < tb->tb_tail)
    {
        tb->tb_head = ii + 1;
    }

    /* Keep the tail and head low */
    if (tb->tb_head >= tb->tb_size)
    {
        tb->tb_head -= tb->tb_size;
        tb->tb_tail -= tb->tb_size;
    }
}

bool tb_strput(struct txtbuf *tb, char *str)
{
    size_t str_len = strlen(str);

    /* Crop the string to the maximum lenght of the buffer */
    if ((str_len + sizeof(char)) > tb->tb_size)
    {
        str_len = tb->tb_size - 1;
    }

    while (tb_free(tb) < (str_len + 1))
    {
        /* Remove strings from the buffer until we can fit it all */
        tb_strtrim(tb);
    }

    /* Put stirng without '\0' */
    if (!tb_put(tb, str, str_len))
    {
        return false;
    }

    /* Patch in "\0" */
    if (!tb_put(tb, "\0", 1))
    {
        return false;
    }

    return true;
}

/* Return number of strings stored in the txtbuf */
int tb_strnum(struct txtbuf *tb)
{
    size_t ii;

    int nstr = 0;

    for (ii = tb->tb_head; ii < tb->tb_tail; ii++)
    {
        if (tb->tb_text[ii % tb->tb_size] == '\0') nstr++;
    }

    return nstr;
}


/* Get the Nth string from the textbuf */
bool tb_strget(struct txtbuf *tb, int index, char *dst, size_t dst_sz)
{
    size_t str_start;
    size_t str_end;
    size_t str_sz;
    size_t tb_off;

    int nstr = 0;

    /* Find the end of the string */
    str_start = tb->tb_head;

    for (str_end = tb->tb_head; str_end < tb->tb_tail; str_end++)
    {
        if (tb->tb_text[str_end % tb->tb_size] == '\0')
        {
            if (nstr == index) break;
            nstr++;

            /* skip this '\0', and start pointing at the beginning of the next string */
            str_start = str_end + 1;
        }
    }
    
    /* Not found? */
    if (str_end == tb->tb_tail) return false;

    /* str_start should point to the first char of the string, while str_end points to the '\0' */
    str_sz = str_end - str_start;

    if (str_sz >= dst_sz)
    {
        str_sz = dst_sz - 1; /* Size of dst, but leave room for the ending '\0' */
    }

    while (str_sz > 0)
    {
        size_t copy_sz;

        tb_off = str_start % tb->tb_size;

        if ((tb_off + str_sz) < tb->tb_size)
        {
            copy_sz = str_sz;
        }
        else 
        {
            copy_sz = tb->tb_size - tb_off;
        }

        memcpy(dst, tb->tb_text + tb_off, copy_sz);
        
        /* Advance pointers by copy_sz positions */
        str_start += copy_sz;
        dst       += copy_sz;      /* Move destination buffer forward              */
        str_sz    -= copy_sz;      /* Decrease the number of bytes left to copy    */
    }

    *dst = '\0';

    return true;
}

/* Same as tb_stget(), except the indexes are reversed (0 - last string) */
bool tb_strlast(struct txtbuf *tb, int index, char *dst, size_t dst_sz)
{
    int nstr;

    nstr = tb_strnum(tb);
    if (index >= nstr)
    { 
        return false;
    }

    nstr = nstr - index - 1;

    return tb_strget(tb, nstr, dst, dst_sz);
}

#if TEST
void tb_dump(struct txtbuf *tb)
{
    printf("tb_head = %u\n", tb->tb_head);
    printf("tb_tail = %u\n", tb->tb_tail);
    printf("tb_size = %u\n", tb->tb_size);
}

int main(void)
{
    char buf[32];
    char tt[3];
    int ii;

    struct txtbuf tb;

    memset(buf, 0, sizeof(buf));

    tb_init(&tb, buf, sizeof(buf));
    tb_strput(&tb, "Hello world");
    printf("nstr = %d\n", tb_strnum(&tb));
    tb_strput(&tb, "Nice to meet ");
    printf("nstr = %d\n", tb_strnum(&tb));
    tb_strput(&tb, "How are you?");
    printf("nstr = %d\n", tb_strnum(&tb));
    tb_strput(&tb, "hh");


    for (ii = 0; tb_strlast(&tb, ii, tt, sizeof(tt)); ii++)
    {
        printf("tt[%d] = %s\n", ii, tt);
    }

    for (ii = 0; ii < sizeof(buf); ii++)
    {
        if (isalpha(buf[ii]) || (buf[ii] >= '0' && buf[ii] <= '9'))
        {
            printf("%c", buf[ii]);
        }
        else if(buf[ii] == '\0')
        {
            printf(".");
        }
        else
        {
            printf("_");
        }
    }

    printf("\n");

}
#endif

