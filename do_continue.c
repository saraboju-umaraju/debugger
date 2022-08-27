#include "debug.h"

int handle_continue(struct argdata *arg)
{
    int status = ptrace(PTRACE_CONT, arg->cpid, NULL, NULL);
    exit_on_error(-1 == status);
    wait_for_child();
    return 0;
}
