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
    return 0;
}

struct bplist *bp_from_address(uint64_t address)
{
    struct bplist *iter = bps;
    for(;iter;) {
        if (iter->address == address)
            return iter;
        iter = iter->next;
    }
    return NULL;
}

static int save_old_content(struct bplist *tmp, uint64_t content)
{
    if (tmp->saved_data == 0)
        tmp->saved_data = content;
    return 0;
}

static uint64_t get_saved_content(struct bplist *tmp)
{
    return tmp->saved_data ;
}

int info_break(void)
{
    struct bplist *iter = bps;
    for(;iter;) {
        printf (" breakpoint %d := 0x%08lx status = %7s\n", iter->index, iter->address, (iter->status ? "enable" : "disable"));
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
    tmp->status = 1;
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

int disable_break(unsigned long addr, struct argdata* arg)
{
    struct bplist *tmp = bp_from_address(addr);
    long data = ptrace(PTRACE_PEEKDATA, arg->cpid, addr, NULL);
    exit_on_error(-1 == data);
    sdebug("peeked data at %lx is %lx\n", addr, data);
    data = get_saved_content(tmp);
    sdebug("poking data at %lx is %lx\n", addr, data);
    int status = ptrace(PTRACE_POKEDATA, arg->cpid, addr, data);
    exit_on_error(-1 == status);
    tmp->status = 0;
    return 0;
}

int enable_break(unsigned long addr, struct argdata* arg)
{
    struct bplist *tmp = bp_from_address(addr);
    long data = ptrace(PTRACE_PEEKDATA, arg->cpid, addr, NULL);
    exit_on_error(-1 == data);
    sdebug("peeked data at %lx is %lx\n", addr, data);
    save_old_content(tmp, data);
    data = data & ~(0xff);
    data = data | (0xcc); //0xcc is int 3 on x86
    sdebug("poking data at %lx is %lx\n", addr, data);
    int status = ptrace(PTRACE_POKEDATA, arg->cpid, addr, data);
    exit_on_error(-1 == status);
    tmp->status = 1;
    return 0;
}
static uint64_t get_enable_disable_data(struct argdata *arg)
{
    sdebug ("%s %s %s \n", arg->v[0], arg->v[1], arg->v[2]);
    if (!arg->v[1]) {
        debug ("missing arg\n");
        return 0;
    }
    unsigned long int index = strtoul(arg->v[1], NULL, 10);
    if (0 == index) {
        debug ("Invalid breakpoint\n");
        return 0;
    }
    return bp_from_index(index);
}
int handle_enable(struct argdata *data)
{
    uint64_t bpindex = 0;
    if (0 == (bpindex = get_enable_disable_data(data))) {
        return -1;
    }
    enable_break(bpindex, data);
    return 0;
}

int handle_disable(struct argdata *data)
{
    uint64_t bpindex = 0;
    if (0 == (bpindex = get_enable_disable_data(data))) {
        return -1;
    }
    disable_break(bpindex, data);
    return 0;
}

int handle_break(struct argdata *data)
{
    struct argdata *arg = (struct argdata*)data;
    sdebug ("%s %s %s \n", arg->v[0], arg->v[1], arg->v[2]);
    if (!arg->v[1]) {
        debug ("break need an arg\n");
        return 1;
    }
    unsigned long int address = strtoul(arg->v[1], NULL, 16);
    exit_on_error(0 == address);
    debug ("tryna break at %lx\n", address);
    bps = bp_add(bps, address);
    enable_break(address, arg);
    return 0;
}

