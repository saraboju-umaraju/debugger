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
#include <fcntl.h>
#include <elf.h>
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
#define str(X) #X

#define SECTION_NAME(X)	((X) == NULL ? "<none>" : \
        ((X)->sh_name >= string_table_length \
         ? "<corrupt>" : string_table + (X)->sh_name))
#define NUM_ELEM(array) 	(sizeof (array) / sizeof ((array)[0]))
#define error debug
#define warn debug
#define _(...) __VA_ARGS__
#include "dwarf.h"
#define xmalloc malloc
#define xrealloc realloc
uint64_t
byte_get (const unsigned char *field, unsigned int size);
unsigned long int
read_leb128 (unsigned char *data, int *length_return, int sign);
void
decode_location_expression (unsigned char * data,
			    unsigned int pointer_size,
			    unsigned long length);

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
int handle_mem(struct argdata *data);
int info_break(void);
uint64_t get_pc(struct argdata *arg);
uint64_t set_pc(struct argdata *arg, uint64_t pc);
int cmd_match(struct cmdlist *iter, char *line);
struct cmdlist *match_cmd(void *data, char *cmd_string);
int process_command(char *line, size_t maxsize, struct argdata*);
int initcmdlist(void);
int wait_for_child(void);
struct bplist *bp_from_address(uint64_t address);
int disable_break(unsigned long addr, struct argdata* arg);
int enable_break(unsigned long addr, struct argdata* arg);
int handle_help(struct argdata *arg);
int do_elf_load();
#endif // __DEBUG_H__
