#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#define debug(...) printf (__VA_ARGS__)
#define sdebug(...)
#define exit_on_error(cond) \
    if (cond) { \
        debug ("exiting on error %s %s %d\n", __FILE__, __func__, __LINE__);\
        exit(1);\
    }

#define CMD_LEN 16
#define LINE_SIZE 80

struct bplist {
    uint64_t address;
    uint64_t saved_data;
    int index;
    struct bplist *next;
};

struct cmdlist {
    char command[CMD_LEN];
    int (*hf)(void *);
    int (*help)(void);
    struct cmdlist *next;
};

struct argdata {
    pid_t cpid;
    char v0[CMD_LEN];
    char v1[CMD_LEN];
    char v2[CMD_LEN];
};

int handle_enable(void *data);
int handle_disable(void *data);
void parent(void);
void child(void);
int handle_continue(void *data);
int handle_break(void *data);
int handle_info(void *data);
int info_break(void);
int handle_enable(void *data);
int cmd_match(struct cmdlist *iter, char *line);
int process_command(char *line, size_t maxsize, struct argdata*);
int initcmdlist(void);
int wait_for_child(void);
#endif // __DEBUG_H__
