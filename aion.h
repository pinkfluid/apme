#ifndef AION_H_INCLUDED
#define AION_H_INCLUDED

#define AION_CHAT_SZ        1024
#define AION_NAME_SZ        64
#define AION_NAME_DEFAULT   "You"

extern bool aion_init(void);
extern bool aion_player_is_self(char *charname);
extern bool aion_group_join(char *charname);
extern bool aion_group_leave(char *charname);
extern void aion_group_disband(void);
extern bool aion_group_apvalue_update(char *charname, uint32_t apval);
extern bool aion_group_apvalue_set(char *charname, uint32_t apval);
extern bool aion_player_chat_cache(char *charname, char *chat);
extern bool aion_player_chat_get(char *charname, int msgnum, char *dst, size_t dst_sz);
extern void aion_player_name_set(char *charname);

#define LANG_ELYOS      1
#define LANG_ASMODIAN   2
#define LANG_KRALL      3
#define LANG_MAU        4
#define LANG_BALAUR     5

extern void aion_translate(char *txt, uint32_t language);
extern void aion_rtranslate(char *txt, uint32_t language);

extern bool aion_group_get_stats(char *stats, size_t stats_sz);
extern bool aion_group_get_aprollrights(char *stats, size_t stats_sz);

#endif // AION_H_INCLUDED
