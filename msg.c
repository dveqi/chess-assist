#include "msg.h"
#include <stdarg.h>
#include <stdio.h>

int msg_verbose_state = 0;

void msg(enum log_level level, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);

    switch (level) {
    case INFO:
        fprintf(stdout, "\033[32mINFO\033[0m: ");
        vfprintf(stdout, fmt, argptr);
        break;
    case ERROR:
        fprintf(stderr, "\033[37m\033[41mERROR\033[0m: ");
        vfprintf(stderr, fmt, argptr);
        break;
    case DEBUG:
        if (msg_verbose_state == 1) {
            fprintf(stderr, "\033[34mDEBUG\033[0m: ");
            vfprintf(stderr, fmt, argptr);
        }
        break;
    }

    va_end(argptr);
}
