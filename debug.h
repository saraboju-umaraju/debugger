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
#include <sys/user.h>
#define debug(...) printf (__VA_ARGS__)
#define sdebug(...)
#define exit_on_error(cond) \
    if (cond) { \
        debug ("exiting on error %s %s %d\n", __FILE__, __func__, __LINE__);\
        exit(1);\
    }

#define CMD_LEN 16
#define LINE_SIZE 80

struct argdata {
    pid_t cpid;
    char **v;
};

struct bplist {
    uint64_t address;
    uint64_t saved_data;
    int index;
    int status;
    struct bplist *next;
};

struct cmdlist {
    char command[CMD_LEN];
    int (*hf)(struct argdata *);
    int (*help)(void);
    struct cmdlist *next;
};

int handle_enable(struct argdata *data);
int process_command(char *line, size_t maxsize, struct argdata *arg);
int handle_disable(struct argdata *data);
void parent(void);
void child(void);
int handle_continue(struct argdata *data);
int handle_break(struct argdata *data);
int handle_info(struct argdata *data);
int handle_register(struct argdata *data);
int handle_enable(struct argdata *data);
int handle_quit(struct argdata *data);
int info_break(void);
int cmd_match(struct cmdlist *iter, char *line);
struct cmdlist *match_cmd(void *data, char *cmd_string);
int process_command(char *line, size_t maxsize, struct argdata*);
int initcmdlist(void);
int wait_for_child(void);
#endif // __DEBUG_H__
