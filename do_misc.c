#include "debug.h"

extern pid_t proc ;
extern struct cmdlist *commands ;

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
                token = gettok (NULL , delim) ;
                if (token) {
                    sdebug ("got %s \n",token );
                    strncpy(arg->v3, token, CMD_LEN);
                }
            }
        }
    }
    arg->cpid = proc;
    return 0;
}

