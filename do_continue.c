#include "debug.h"

int handle_continue(void *data)
{
    struct argdata *arg = (struct argdata*)data;
    int status = ptrace(PTRACE_CONT, arg->cpid, NULL, NULL);
    exit_on_error(-1 == status);
    wait_for_child();
    return 0;
}
