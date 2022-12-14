#include "debug.h"

#if defined(FREAK)
struct user_regs_struct
{
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rax;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long orig_rax;
    unsigned long rip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long rsp;
    unsigned long ss;
    unsigned long fs_base;
    unsigned long gs_base;
    unsigned long ds;
    unsigned long es;
    unsigned long fs;
    unsigned long gs;
};
#endif

enum registers {
    r15,
    r14,
    r13,
    r12,
    rbp,
    rbx,
    r11,
    r10,
    r9,
    r8,
    rax,
    rcx,
    rdx,
    rsi,
    rdi,
    orig_rax,
    rip,
    cs,
    eflags,
    rsp,
    ss,
    fs_base,
    gs_base,
    ds,
    es,
    fs,
    gs
};

struct register_struct {
    enum registers er;
    char name[CMD_LEN];
    uint64_t dwarf_number;
};


struct register_struct x86_registers [] = {
 [r15] =          {r15     ,     "r15",         15},
 [r14] =          {r14     ,     "r14",         14},
 [r13] =          {r13     ,     "r13",         13},
 [r12] =          {r12     ,     "r12",         12},
 [rbp] =          {rbp     ,     "rbp",         6},
 [rbx] =          {rbx     ,     "rbx",         3},
 [r11] =          {r11     ,     "r11",         11},
 [r10] =          {r10     ,     "r10",         10},
 [r9] =           {r9      ,     "r9" ,         9},
 [r8] =           {r8      ,     "r8" ,         8},
 [rax] =          {rax     ,     "rax",         0},
 [rcx] =          {rcx     ,     "rcx",         2},
 [rdx] =          {rdx     ,     "rdx",         1},
 [rsi] =          {rsi     ,     "rsi",         4},
 [rdi] =          {rdi     ,     "rdi",         5},
 [orig_rax] =     {orig_rax,     "orig_rax",    -1},
 [rip] =          {rip     ,     "rip",         -1},
 [cs] =           {cs      ,     "cs",          51},
 [eflags] =       {eflags  ,     "eflags",      49},
 [rsp] =          {rsp     ,     "rsp",         7},
 [ss] =           {ss      ,     "ss",          52},
 [fs_base] =      {fs_base ,     "fs_base",     58},
 [gs_base] =      {gs_base ,     "gs_base",     59},
 [ds] =           {ds      ,     "ds",          53},
 [es] =           {es      ,     "es",          50},
 [fs] =           {fs      ,     "fs",          54},
 [gs] =           {gs      ,     "gs",          55}
 };

static int get_register_struct(char *rstr, struct register_struct x86_registers[], size_t len)
{
    int k = 0;
    for(;k < len; k++) {
        sdebug ("%8s = %3ld %d\n", x86_registers[k].name, x86_registers[k].dwarf_number, x86_registers[k].er);
        sdebug ("comparing %s %s\n", rstr, x86_registers[k].name);
        if (0 == strcmp(rstr, x86_registers[k].name)) {
            return k;
        }
    }
    return -1;
}

static void dump_registers(struct user_regs_struct *regs)
{
    int k = 0;
    unsigned long *local_regs = (unsigned long*)regs;
    size_t len = sizeof(x86_registers)/sizeof(x86_registers[0]);
    for(;k < len; k++) {
        debug ("%8s = %3ld %d %lx\n", x86_registers[k].name, x86_registers[k].dwarf_number, x86_registers[k].er, local_regs[k]);
    }
}

static int read_registers(pid_t cpid, struct user_regs_struct *regs)
{
    int status = ptrace(PTRACE_GETREGS, cpid, NULL, regs);
    exit_on_error(-1 == status);
    return 0;
}

static int write_registers(pid_t cpid, struct user_regs_struct *regs)
{
    int status = ptrace(PTRACE_SETREGS, cpid, NULL, regs);
    exit_on_error(-1 == status);
    return 0;
}

uint64_t get_pc(pid_t cpid)
{
    struct user_regs_struct regs = {0};
    read_registers(cpid, &regs);
    return regs.rip;
}

uint64_t set_pc(pid_t cpid, uint64_t pc)
{
    struct user_regs_struct regs = {0};
    read_registers(cpid, &regs);
    regs.rip = pc;
    write_registers(cpid, &regs);
    return 0;
}

int handle_register(struct argdata *data)
{
    struct argdata *arg = (struct argdata*)data;
    sdebug ("%s %s %s \n", arg->v[0], arg->v[1], arg->v[2]);
    struct user_regs_struct regs = {0};
    read_registers(arg->cpid, &regs);
    struct register_struct *tmp = NULL;
    int index = -1;

    if (arg->v[1] == NULL) {
        dump_registers(&regs);
    } else if (arg->v[1][0] == 'r') {
        index = get_register_struct(arg->v[2], x86_registers, 27);
        exit_on_error(-1 == index);
        tmp = &x86_registers[index];
        debug ("0x%lx \n", ((unsigned long*)&regs)[tmp->er]);
    } else if(arg->v[1][0] == 'w') {
        index = get_register_struct(arg->v[2], x86_registers, 27);
        exit_on_error(-1 == index);
        tmp = &x86_registers[index];
        unsigned long int data = strtoul(arg->v[3], NULL, 16);
        sdebug ("0x%lx \n", ((unsigned long*)&regs)[tmp->er]);
        ((unsigned long*)&regs)[tmp->er] = data;
        write_registers(arg->cpid, &regs);
    }
    return 0;
}

