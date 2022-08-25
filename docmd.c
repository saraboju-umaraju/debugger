#include "debug.h"

void handle_continue(void *data)
{
    pid_t *cpid = (pid_t*)data;
    ptrace(PTRACE_CONT, *cpid, NULL, NULL);
    wait_for_child();
}


