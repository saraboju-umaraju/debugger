#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#define debug(...) printf (__VA_ARGS__)
#define exit_on_error(cond) \
    if (cond) { \
        debug ("exiting on error %s %s %d\n", __FILE__, __func__, __LINE__);\
        exit(1);\
    }

#define CMD_LEN 16
#define LINE_SIZE 80

struct cmdlist {
    char command[CMD_LEN];
    void (*hf)(void *);
    struct cmdlist *next;
};

void parent(void);
void child(void);
void handle_continue(void *data);
int cmd_match(struct cmdlist *iter, char *line);
int initcmdlist(void);
int wait_for_child(void);
#endif // __DEBUG_H__
