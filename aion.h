#ifndef AION_H_INCLUDED
#define AION_H_INCLUDED

extern bool aion_init(void);
extern bool aion_player_is_self(char *charname);
extern bool aion_group_join(char *charname);
extern bool aion_group_leave(char *charname);
extern void aion_group_disband(void);
extern bool aion_group_apvalue_update(char *charname, uint32_t apval);

#endif // AION_H_INCLUDED
