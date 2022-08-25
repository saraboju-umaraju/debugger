#include "debug.h"

struct bplist *bps = NULL;

static uint64_t bp_from_index(uint64_t index)
{
    struct bplist *iter = bps;
    for(;iter;) {
        if (iter->index == index)
            return iter->address;
        iter = iter->next;
    }
    return -1;
}

int info_break(void)
{
    struct bplist *iter = bps;
    for(;iter;) {
        printf (" breakpoint %d := 0x%08lx\n", iter->index, iter->address);
        iter = iter->next;
    }
    return 0;
}

static struct bplist *add_node(uint64_t bp)
{
    struct bplist *tmp = (struct bplist*) malloc(sizeof(struct bplist));
    exit_on_error(tmp == NULL);
    tmp->address = bp;
    tmp->index = 0;
    tmp->next = NULL;
    return tmp;
}

struct bplist *bp_add(struct bplist *head, uint64_t bp)
{
    struct bplist *newnode = add_node(bp);
    if (NULL == head) {
        newnode->index = 1;
        return newnode;
    }else {
        newnode->next = head;
        newnode->index = head->index + 1;
        return newnode;
    }
}

static int disable_break(unsigned long addr, struct argdata* arg)
{
    long data = ptrace(PTRACE_PEEKDATA, arg->cpid, addr, NULL);
    exit_on_error(-1 == data);
    debug("peeked data at %lx is %lx\n", addr, data);
    //uint64_t saved_data = data;
    data = data & ~(0xff);
    data = data | (0xe8); //0xcc is int 3 on x86
    debug("poking data at %lx is %lx\n", addr, data);
    int status = ptrace(PTRACE_POKEDATA, arg->cpid, addr, data);
    exit_on_error(-1 == status);
    return 0;
}

static int enable_break(unsigned long addr, struct argdata* arg)
{
    long data = ptrace(PTRACE_PEEKDATA, arg->cpid, addr, NULL);
    exit_on_error(-1 == data);
    debug("peeked data at %lx is %lx\n", addr, data);
    //uint64_t saved_data = data;
    data = data & ~(0xff);
    data = data | (0xcc); //0xcc is int 3 on x86
    debug("poking data at %lx is %lx\n", addr, data);
    int status = ptrace(PTRACE_POKEDATA, arg->cpid, addr, data);
    exit_on_error(-1 == status);
    return 0;
}
static uint64_t get_enable_disable_data(void *data)
{
    struct argdata *arg = (struct argdata*)data;
    debug ("%s %s %s \n", arg->v0, arg->v1, arg->v2);
    if (!arg->v1[0]) {
        debug ("subfunction would need an argument\n");
        return 1;
    }
    unsigned long int address = strtoul(arg->v1, NULL, 10);
    if (0 == address) {
        debug ("Invalid breakpoint\n");
        return 0;
    }
    return bp_from_index(address);
}
int handle_enable(void *data)
{
    struct argdata *arg = (struct argdata*)data;
    debug ("%s %s %s \n", arg->v0, arg->v1, arg->v2);
    uint64_t bpindex = 0;
    if (0 == (bpindex = get_enable_disable_data(data))) {
        return -1;
    }
    enable_break(bpindex, data);
    return 0;
}

int handle_disable(void *data)
{
    uint64_t bpindex = 0;
    if (0 == (bpindex = get_enable_disable_data(data))) {
        return -1;
    }
    disable_break(bpindex, data);
    return 0;
}

int handle_break(void *data)
{
    struct argdata *arg = (struct argdata*)data;
    debug ("%s %s %s \n", arg->v0, arg->v1, arg->v2);
    if (!arg->v1[0]) {
        debug ("break would need an argument\n");
        return 1;
    }
    unsigned long int address = strtoul(arg->v1, NULL, 16);
    exit_on_error(0 == address);
    debug ("tryna break at %lx\n", address);
    bps = bp_add(bps, address);
    enable_break(address, arg);
    return 0;
}

