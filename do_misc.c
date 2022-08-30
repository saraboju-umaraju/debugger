#include "debug.h"

extern pid_t proc ;
extern struct cmdlist *commands ;

int handle_help(struct argdata *arg)
{
    struct cmdlist *looper = commands;
    debug ("Available commands on cdb\n");
    for(; looper; looper = looper->next) {
        printf(" %10s\n", looper->command);
    }
    return 0;
}


struct cmdlist *match_cmd(void *data, char *cmd_string)
{
    int len = strlen(cmd_string);
    int i = 1;
    int k = 0;
    struct cmdlist *looper = commands;
    struct cmdlist *tmp = NULL;
    for(; (i <= len) && (i < CMD_LEN); i++) {
        for(k = 0, tmp = NULL, looper = commands; looper; looper = looper->next) {
            sdebug ("processing %s with %d\n", looper->command, i);
            if (0 == strncmp(looper->command, cmd_string, i)) {
                if(!k++) {
                    tmp = looper;
                    if (0 == strcmp(looper->command, cmd_string)) {
                        return tmp;
                    }
                } else {
                    tmp = NULL;
                    break;
                }
            }
        }
        if (tmp) {
            if (!cmd_string[i]) {
                return tmp;
            }
        }
    }
    return NULL;
}

static char* gettok(char *str , char* str2)
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


static void remove_nl(char *line)
{
    line[strlen(line)-1] = '\0';
}

int process_command(char *line, size_t maxsize, struct argdata *arg)
{
    char *delim = " ";
    char *token = NULL;
    int i = 0;

    remove_nl(line);

    token = gettok(line , delim) ;
    // just the command name
    while (token) {
        sdebug ("got %s \n",token );
        arg->v = (char **) realloc (arg->v, sizeof(char *)*(i+2));
        arg->v[i] = (char *) malloc (sizeof(char) * CMD_LEN);
        strncpy(arg->v[i++], token, CMD_LEN);
        sdebug ("got %s \n",arg->v[i-1]);
        token = gettok (NULL , delim) ;
    }
    // Who even gave me a job in Nvidia lol :D
    arg->cpid = proc;
    return 0;
}

