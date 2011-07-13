#ifndef AION_H_INCLUDED
#define AION_H_INCLUDED

extern bool aion_init(void);
extern bool aion_player_is_self(char *charname);
extern bool aion_group_join(char *charname);
extern bool aion_group_leave(char *charname);
extern void aion_group_disband(void);
extern bool aion_group_apvalue_update(char *charname, uint32_t apval);
extern bool aion_group_apvalue_set(char *charname, uint32_t apval);

#define LANG_ELYOS      1
#define LANG_ASMODIAN   2
#define LANG_KRALL      3
#define LANG_MAU        4
#define LANG_BALAUR     5

extern void aion_translate(char *txt, uint32_t language);

extern bool aion_group_get_stats(char *stats, size_t stats_sz);
extern bool aion_group_get_aprollrights(char *stats, size_t stats_sz);

#endif // AION_H_INCLUDED
