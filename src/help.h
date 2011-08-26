#ifndef HELP_H_INCLUDED
#define HELP_H_INCLUDED

extern const char *help_chatlog_warning;
extern const char *help_chatlog_enabled;
extern const char *help_chatlog_enable_error;

extern void help_cmd(char *cmd, char *help, size_t help_sz);
extern void help_usage(char *usage, size_t usage_sz);

#endif /* HELP_H_INCLUDED */

