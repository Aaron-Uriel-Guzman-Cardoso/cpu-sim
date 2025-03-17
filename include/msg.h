#ifndef MSG_H
#define MSG_H

#include <ncurses.h>
#include <stdint.h>

enum log_level { LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR };

int32_t msg_init(void);
int32_t list_init(void);
int32_t msg_log(enum log_level level, const char *str);

#endif
