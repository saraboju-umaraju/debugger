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
#include <signal.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/user.h>
#define debug(...) printf (__VA_ARGS__)
#define sdebug(...)
#define exit_on_error(cond) \
    if (cond) { \
        perror("");\
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

struct attr_data
{
    unsigned long uvalue;
    unsigned char *svalue;
    unsigned long indirect;
};
uint64_t
byte_get (const unsigned char *field, unsigned int size);
unsigned long int
read_leb128 (unsigned char *data, int *length_return, int sign);
void
decode_location_expression (unsigned char * data,
			    unsigned int pointer_size,
			    unsigned long length);

unsigned char *
display_block (unsigned char *data, unsigned long length);
struct argdata {
    pid_t cpid;
    char **v;
};
void *
get_data (void *var, int fd, long offset, size_t size, const char *reason);
elf64_shdr *
find_section (const char * name);
int display_line_info_uma (int file);
int display_line_info_uma_2 (int file);

unsigned char *
read_and_display_attr (unsigned long attribute,
		       unsigned long form,
		       unsigned char *data,
		       unsigned long cu_offset,
		       unsigned long pointer_size,
		       unsigned long offset_size,
		       int dwarf_version);
unsigned char *
read_attr (unsigned long attribute,
		       unsigned long form,
		       unsigned char *data,
		       unsigned long cu_offset,
		       unsigned long pointer_size,
		       unsigned long offset_size,
		       int dwarf_version,
               struct attr_data *ptr);

unsigned char *
read_and_display_attr_value (unsigned long attribute,
			     unsigned long form,
			     unsigned char *data,
			     unsigned long cu_offset,
			     unsigned long pointer_size,
			     unsigned long offset_size,
			     int dwarf_version);
int
display_debug_lines (elf64_shdr *section,
		     unsigned char *start, int file);
int
display_debug_frames (elf64_shdr *section,
		      unsigned char *start,
		      int file );
const char *
fetch_indirect_string (unsigned long offset);
typedef struct abbrev_attr
{
    unsigned long attribute;
    unsigned long form;
    unsigned char *saved_addr;
    struct abbrev_attr *next;
}
abbrev_attr;
void try_display_line_info(unsigned long could_be_bp);

typedef struct abbrev_entry
{
    unsigned long entry;
    unsigned long saved_entry;
    unsigned long tag;
    int children;
    struct abbrev_attr *first_attr;
    struct abbrev_attr *last_attr;
    struct abbrev_entry *next;
}
abbrev_entry;

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
int handle_loop(struct argdata *arg);
int handle_step(struct argdata *arg);

int do_run_cmd(struct argdata *arg);
int handle_enable(struct argdata *data);
int process_command(char *line, size_t maxsize, struct argdata *arg);
int handle_disable(struct argdata *data);
void parent(pid_t);
void child(void);
int handle_continue(struct argdata *data);
int handle_break(struct argdata *data);
int handle_info(struct argdata *data);
int handle_register(struct argdata *data);
int handle_enable(struct argdata *data);
int handle_quit(struct argdata *data);
int handle_mem(struct argdata *data);
int info_break(void);
uint64_t get_pc(pid_t cpid);
uint64_t set_pc(pid_t cpid, uint64_t pc);
int cmd_match(struct cmdlist *iter, char *line);
struct cmdlist *match_cmd(void *data, char *cmd_string);
int process_command(char *line, size_t maxsize, struct argdata*);
int initcmdlist(void);
int wait_for_child(pid_t);
struct bplist *bp_from_address(uint64_t address);
int disable_break(unsigned long addr, struct argdata* arg);
int enable_break(unsigned long addr, struct argdata* arg);
int handle_help(struct argdata *arg);
int do_elf_load();
int handle_elf(struct argdata *arg);
void free_vector(struct argdata *arg);
#endif // __DEBUG_H__
