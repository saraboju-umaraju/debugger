#include "debug.h"

void child()
{
    debug ("congratulations it's a child\n");
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    char *argv[] = {"./tracee", NULL};
    execv("./tracee", argv);
}
