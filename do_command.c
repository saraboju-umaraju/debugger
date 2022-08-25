#include "debug.h"

extern pid_t proc ;

char* gettok(char *str , char* str2)
{
    int var = 0;
    int var2 = 0;
    static char *ptr ;

    if ( str == NULL ) {
        str = ptr;
        if ( ptr == NULL )
            return NULL;
    }

    while(*(str + var)) {    //first while
        var2 = 0;
        while (*(str2 + var2) ) {   //second while
            if (*(str + var) == *(str2 + var2) )	{
                *(str + var ) = '\0';
                var2 =  0 ;
                while (1) {
                    if (*(str2 + var2) == *(str + var + 1)) {
                        *(str + var+ 1 ) = '\0';
                        var++ ;
                        var2 = 0;
                    } else {
                        var2++;
                    }

                    if (!*(str2 + var2)) {
                        break;
                    }
                }
                ptr = (str + var+ 1);
                if ( ! *( str ) ) {
                    continue;
                }
                return str;
                break ;
            }
            var2 ++;
        }
        var ++ ;

    } // first while ends

    ptr = NULL ;

    return str;

}

struct cmdlist *commands = NULL;

static void remove_nl(char *line)
{
    line[strlen(line)-1] = '\0';
}

int process_command(char *line, size_t maxsize, struct argdata *arg)
{
    char *delim = " ";
    char *token = NULL;

    remove_nl(line);

    token = gettok(line , delim) ;
    // Who even gave me a job in Nvidia lol :D
    if (token) {
        sdebug ("got %s \n",token );
        strncpy(arg->v0, token, CMD_LEN);
        token = gettok (NULL , delim) ;
        if (token) {
            sdebug ("got %s \n",token );
            strncpy(arg->v1, token, CMD_LEN);
            token = gettok (NULL , delim) ;
            if (token) {
                sdebug ("got %s \n",token );
                strncpy(arg->v2, token, CMD_LEN);
            }
        }
    }
    arg->cpid = proc;
    return 0;
}

int cmd_match(struct cmdlist *iter, char *line)
{
    sdebug("%s %s\n", iter->command, line);
    sdebug("%zu %zu\n", strlen(iter->command), strlen(line));
    return ( 0 == strcmp(iter->command, line));
}

static struct cmdlist *add_node(const char *cmd, int (*handle_fun)(void *), int (*help_fun)(void))
{
    struct cmdlist *tmp = (struct cmdlist*) malloc(sizeof(struct cmdlist));
    exit_on_error(tmp == NULL);
    strncpy(tmp->command, cmd, CMD_LEN);
    tmp->hf = handle_fun;
    tmp->help = help_fun;
    tmp->next = NULL;
    return tmp;
}

struct cmdlist *cmd_add(struct cmdlist *head, const char *cmd, int (*handle_fun)(void *), int (*help_fun)(void))
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
    return 0;
}
