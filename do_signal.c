#include "debug.h"
#include <signal.h>
#define TRAP_BRKPT  1   /* process breakpoint */
#define TRAP_TRACE  2   /* process trace trap */
#define TRAP_BRANCH     3   /* process taken branch trap */
#define TRAP_HWBKPT     4   /* hardware breakpoint/watchpoint */
#define TRAP_UNK    5   /* undiagnosed trap */
#define TRAP_PERF   6   /* perf event with sigtrap=1 */

void handle_trap(siginfo_t *info, pid_t cpid)
{
    sdebug ("si_code %d!!\n", info->si_code);
    switch(info->si_code) {
        case SI_KERNEL :
        case TRAP_BRKPT:
            {
                uint64_t could_be_bp = get_pc(cpid);
                could_be_bp -= 1;

                struct bplist *bp = bp_from_address(could_be_bp);
                if (bp) {
                    set_pc(cpid, could_be_bp);
                }
                could_be_bp = get_pc(cpid);
                debug ("breakpoint at := %lx\n", could_be_bp);
                try_display_line_info(could_be_bp);
            }
            break;
        case SI_USER :
            {
            }
            break;
        case TRAP_TRACE :
            {
                uint64_t could_be_bp = get_pc(cpid);
                debug ("trap trace RIP = %lx\n", could_be_bp);
                try_display_line_info(could_be_bp);
            }
            break;
        default:
            debug ("Not sure %d!!\n", info->si_code);
    }
}

siginfo_t get_signal_info(pid_t cpid, siginfo_t *info)
{
    memset(info, 0, sizeof(siginfo_t));
    long status = ptrace(PTRACE_GETSIGINFO, cpid, NULL, info);
    exit_on_error(-1 == status);
}

int wait_for_child(pid_t cpid)
{
    int status = 0;
    waitpid(cpid, &status, 0);
    if (WIFEXITED(status)) {
        debug ("looks like child had terminated normally\n");
        exit(0);
    }
    siginfo_t info;
    get_signal_info(cpid, &info);
    switch (info.si_signo) {
        case SIGSEGV:
            debug ("crashed with %d, exiting\n", info.si_code);
            exit(info.si_code);
            break;
        case SIGTRAP:
            // trap is breakpoint, handle it.
            handle_trap(&info, cpid);
            break;
    }
#if 0
    if (WIFEXITED(status)) {
        debug ("looks like child had terminated normally\n");
        exit(0);
    } else if (WSTOPSIG(status)) {
        sdebug ("child is stopped by a signal = %d\n", WSTOPSIG(status));
    }
#endif
    return 0;
}
