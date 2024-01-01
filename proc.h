#ifndef _LIBUCI_PROC_H_
#define _LIBUCI_PROC_H_

#include <stdio.h>

int popen_with_fd(const char* command, FILE** process_stdin, FILE** process_stdout);

#endif
