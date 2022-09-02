#include "debug.h"

int handle_continue(struct argdata *arg)
{
    uint64_t could_be_bp = get_pc(arg->cpid);
    //
    // int 3 only a byte, so lets move back a byte and sent cont signal to the child.
    could_be_bp -= 1;

    struct bplist *bp = bp_from_address(could_be_bp);
    if (bp) {
        set_pc(arg->cpid, could_be_bp);
        disable_break(could_be_bp, arg);
        int status = ptrace(PTRACE_SINGLESTEP, arg->cpid, NULL, NULL);
        exit_on_error(-1 == status);
        wait_for_child(arg->cpid);
        enable_break(could_be_bp, arg);
    }
    int status = ptrace(PTRACE_CONT, arg->cpid, NULL, NULL);
    exit_on_error(-1 == status);
    wait_for_child(arg->cpid);
    return 0;
}
