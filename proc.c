#include "proc.h"

#include <unistd.h>
#include <stdlib.h>

#define IS_CHILD_PROCESS(pid) (pid == 0)

int popen_with_fd(const char* command, FILE** process_stdin, FILE** process_stdout)
{
    int process_streams[2], our_streams[2];

    if (pipe(process_streams) == -1 || pipe(our_streams) == -1) {
        return -1;
    }

    int pid = fork();
    if (pid == -1) {
        return -1;
    }

    if (IS_CHILD_PROCESS(pid)) {
        close(process_streams[1]);
        close(our_streams[0]);
        dup2(process_streams[0], STDIN_FILENO);
        dup2(our_streams[1], STDOUT_FILENO);
        execlp(command, command, NULL);
        exit(1);
    } else {
        close(process_streams[0]);
        close(our_streams[1]);
        *process_stdout = fdopen(our_streams[0], "r");
        *process_stdin = fdopen(process_streams[1], "w");
        setvbuf(*process_stdin, NULL, _IONBF, 0);
    }

    return 0;
}
