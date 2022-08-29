#include "debug.h"

#if defined(DUMP_MAP)
static int dump_map()
{
    FILE *fptr = fopen("/proc/self/maps", "r");
    exit_on_error (fptr == NULL);
    char c = fgetc(fptr);
    while (c != EOF) {
        printf ("%c", c);
        c = fgetc(fptr);
    }

    fclose(fptr);
    return 0;
}
#endif

void child()
{
#if defined(DUMP_MAP)
    dump_map();
#endif
    sdebug ("child = %d\n", getpid());
    int status = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    exit_on_error(-1 == status);
    char *argv[] = {"./do_me", NULL};
    execv("./do_me", argv);
}
