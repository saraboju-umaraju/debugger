#include "debug.h"

static int write_to_mem(struct argdata *arg, size_t len)
{
    unsigned long int address = strtoul(arg->v[2], NULL, 16);
    unsigned long int data = strtoul(arg->v[3], NULL, 16);
    long status = ptrace(PTRACE_POKEDATA, arg->cpid, address, data);
    exit_on_error(-1 == status);
    debug("poking data at %lx is %lx\n", address, data);
    return 0;
}

static int read_from_mem(struct argdata *arg, size_t len)
{
    unsigned long int address = strtoul(arg->v[2], NULL, 16);
    long data = ptrace(PTRACE_PEEKDATA, arg->cpid, address, NULL);
    exit_on_error(-1 == data);
    debug("peeked data at %lx is %lx\n", address, data);
    return 0;
}

int handle_mem(struct argdata *data)
{
    struct argdata *arg = (struct argdata*)data;
    sdebug ("%s %s %s \n", arg->v[0], arg->v[1], arg->v[2]);

    if (arg->v[1] == NULL || arg->v[2] == NULL) {
        debug ("usage : mem r/w 0xaddr\n");
    } else if (arg->v[1][0] == 'r') {
        read_from_mem(data, 8);
    } else if (arg->v[1][0] == 'w') {
        if (arg->v[3] == NULL) {
            debug ("usage : mem w 0xaddr 0xdata\n");
            return -1;
        }
        write_to_mem(data, 8);
    }
    return 0;
}


