#include <stdlib.h>

#include "event.h"

static event_callback_t *event_process_cb = NULL;

void event_register(event_callback_t *event_cb)
{
    event_process_cb = event_cb;
}

void event_signal(enum event_type event)
{
    if (event_process_cb == NULL) return;

    event_process_cb(event);
}
