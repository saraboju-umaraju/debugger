#include "debug.h"

extern struct cmdlist *commands ;
extern pid_t proc ;

static void do_run_cmd(char *line)
{
    struct cmdlist *iter = commands;
    for(;iter;) {
        if (cmd_match(iter, line)) {
            iter->hf((void*)&proc);
            break;
        }
        iter = iter->next;
    }
}
static void stripnewline(char *line)
{
    line[strlen(line)-1] = '\0';
}

static char* usercommand()
{
    char *line = (char *) malloc (LINE_SIZE);
    if (NULL == line) {
        printf("low on memory\n");
        exit(1);
    }

    while (1) {
        printf("$_ ");
        if (NULL != fgets(line, LINE_SIZE, stdin)) {
            stripnewline(line);
            return line;
        } else {
            printf ("\nlet me know what to do\n");
        }
    }
    return NULL;
}

int wait_for_child()
{
    int status = 0;
    waitpid(proc, &status, 0);
    if (WIFEXITED(status)) {
        debug ("looks like child had terminated normally\n");
        exit(0);
    } else if (WSTOPSIG(status)) {
        debug ("child is stopped by a signal = %d\n", WSTOPSIG(status));
    }
    return 0;
}

void parent(void)
{
    wait_for_child();

    debug ("parent is doing its job\n");

    initcmdlist();

    char *line = NULL;
    while ((line = usercommand())) {
        do_run_cmd(line);
        free(line);
        line = NULL;
    }

}

