#include "proc.h"
#include <unistd.h>
#include <stdlib.h>

#define IS_CHILD_PROCESS(pid) pid == 0

int popen_with_fd(const char* command, FILE** process_stdin, FILE** process_stdout)
{
    /* stdin/stdout file descriptors */
    int process_streams[2] = { -1, -1 };
    int our_streams[2] = { -1, -1 };

    pipe(process_streams);
    pipe(our_streams);

    int pid = fork();

    if (IS_CHILD_PROCESS(pid)) {
        dup2(process_streams[0], STDIN_FILENO);
        dup2(our_streams[1], STDOUT_FILENO);
        execlp(command, command, NULL);
        exit(1);
    } else {
        *process_stdin = fdopen(our_streams[0], "r");
        *process_stdout = fdopen(process_streams[1], "w");
        setvbuf(*process_stdout, NULL, _IONBF, BUFSIZ);
    }

    return 0;
}
