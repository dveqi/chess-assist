#ifndef _LIBUCI_MSG_H_
#define _LIBUCI_MSG_H_

enum log_level
{
    INFO,
    ERROR,
    DEBUG
};

extern int msg_verbose_state;

void msg(enum log_level level, const char* fmt, ...);

#endif
