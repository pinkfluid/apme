#ifndef GROUP_H_INCLUDED
#define GROUP_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

struct group_member
{
    char        *gm_name;   /* Player name      */
    uint32_t    gm_ap;      /* Acculumated AP   */
};

extern bool group_init(int nmembers, char *names[]);
extern void group_stats(void);
extern void group_ap_update(char *name, uint32_t ap);
extern void group_ap_eligible(void);

#endif // GROUP_H_INCLUDED

