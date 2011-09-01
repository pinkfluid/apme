#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

enum event_type
{
    EVENT_AION_GROUP_UPDATE     = 0,    /* Group has been updated       */
    EVENT_AION_AP_UPDATE        = 1,    /* AP value of a member update  */
    EVENT_AION_INVENTORY_FULL   = 2,    /* Somebody has inventory full  */
    EVENT_AION_LOOT_RIGHTS      = 3,    /* New loot rights calculated   */
};

typedef void event_callback_t(enum event_type ev);

extern void event_register(event_callback_t *event_cb);
extern void event_signal(enum event_type event);

#endif /* EVENT_H_INCLUDED */
