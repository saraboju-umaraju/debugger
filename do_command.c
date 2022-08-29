#include "debug.h"

struct cmdlist *commands = NULL;


int cmd_match(struct cmdlist *iter, char *line)
{
    sdebug("%s %s\n", iter->command, line);
    sdebug("%zu %zu\n", strlen(iter->command), strlen(line));
    return ( 0 == strcmp(iter->command, line));
}

static struct cmdlist *add_node(const char *cmd, int (*handle_fun)(struct argdata *), int (*help_fun)(void))
{
    struct cmdlist *tmp = (struct cmdlist*) malloc(sizeof(struct cmdlist));
    exit_on_error(tmp == NULL);
    strncpy(tmp->command, cmd, CMD_LEN);
    tmp->hf = handle_fun;
    tmp->help = help_fun;
    tmp->next = NULL;
    return tmp;
}

struct cmdlist *cmd_add(struct cmdlist *head, const char *cmd, int (*handle_fun)(struct argdata *), int (*help_fun)(void))
{
    sdebug ("adding %10s to the command list\n", cmd);
    struct cmdlist *newnode = add_node(cmd, handle_fun, help_fun);
    if (NULL == head) {
        return newnode;
    }else {
        newnode->next = head;
        return newnode;
    }
}

int initcmdlist(void)
{
    commands = cmd_add(commands, "continue", &handle_continue, NULL);
    commands = cmd_add(commands, "break", &handle_break, info_break);
    commands = cmd_add(commands, "enable", &handle_enable, NULL);
    commands = cmd_add(commands, "disable", &handle_disable, NULL);
    commands = cmd_add(commands, "info", &handle_info, NULL);
    commands = cmd_add(commands, "register", &handle_register, NULL);
    commands = cmd_add(commands, "quit", &handle_quit, NULL);
    commands = cmd_add(commands, "memory", &handle_mem, NULL);
    return 0;
}
