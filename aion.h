#ifndef AION_H_INCLUDED
#define AION_H_INCLUDED

extern bool aion_init(void);
extern bool aion_group_join(char *charname);
extern bool aion_group_leave(char *charname);
extern bool aion_group_ap_update(char *charname, uint32_t apval);

#endif // AION_H_INCLUDED
