#ifndef CHATLOG_H_INCLUDED
#define CHATLOG_H_INCLUDED


#define CHATLOG_PREFIX_LEN      20              /* Length of the prefix for each
                                                    chatlog line, the timestamp */
#define CHATLOG_FILENAME        "Chat.log"
#define CHATLOG_CHAT_SZ         1024
#define CHATLOG_NAME_SZ         64
#define CHATLOG_ITEM_SZ         64

extern bool chatlog_init(void);
extern bool chatlog_poll(void);

#endif
