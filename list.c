#include "debug.h"

struct cmdlist *commands = NULL;

int cmd_match(struct cmdlist *iter, char *line)
{
    printf("%s %s\n", iter->command, line);
    printf("%zu %zu\n", strlen(iter->command), strlen(line));
    return ( 0 == strcmp(iter->command, line));
}

struct cmdlist *add_node(const char *cmd, void (*handle_fun)(void *))
{
    struct cmdlist *tmp = (struct cmdlist*) malloc(sizeof(struct cmdlist));
    exit_on_error(tmp == NULL);
    strncpy(tmp->command, cmd, CMD_LEN);
    tmp->hf = handle_fun;
    tmp->next = NULL;
    return tmp;
}

struct cmdlist *cmd_add(struct cmdlist *head, const char *cmd, void (*handle_fun)(void *))
{
    struct cmdlist *newnode = add_node(cmd, handle_fun);
    if (NULL == head) {
        return newnode;
    }else {
        newnode->next = head;
        return newnode;
    }
}

int initcmdlist(void)
{
    commands = cmd_add(commands, "continue", &handle_continue);
    return 0;
}
