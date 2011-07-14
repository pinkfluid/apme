#ifndef ITEM_H_INCLUDED
#define ITEM_H_INCLUDED

#include <stdint.h>

struct item
{
    uint32_t    item_id;
    char        *item_name;
    uint32_t    item_ap;
};

extern struct item* item_find(uint32_t itemid);
extern struct item* item_find_name(char *item_name);

#endif // ITEM_H_INCLUDED
