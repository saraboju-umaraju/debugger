#include "debug.h"

extern struct cmdlist *commands ;
extern pid_t proc ;

static int do_run_cmd(struct argdata *arg)
{
    struct cmdlist *tmp = match_cmd(arg, arg->v[0]);
    if (NULL == tmp) {
        debug ("command not found := %s\n", arg->v[0]);
        debug ("Run help for available commands\n");
        return 1;
    } else {
        return tmp->hf((void*)arg);
    }
    return 0;
}

static struct argdata* usercommand()
{
    char *line = (char *) malloc (LINE_SIZE);
    exit_on_error(NULL == line);
    struct argdata *arg = NULL;

    while (1) {
        printf("$_ ");
        if (NULL != fgets(line, LINE_SIZE, stdin)) {
            if (line[0] == '\n') {
                continue;
            }
            arg = (struct argdata*) malloc(sizeof(struct argdata));
            sdebug ("after allocation = %p\n", arg);
            memset(arg, 0, sizeof(struct argdata));
            exit_on_error(NULL == arg);
            process_command(line, LINE_SIZE, arg);
        } else {
            printf ("\nlet me know what to do\n");
        }
        free(line);
        return arg;
    }
}

int wait_for_child()
{
    int status = 0;
    waitpid(proc, &status, 0);
    if (WIFEXITED(status)) {
        debug ("looks like child had terminated normally\n");
        exit(0);
    } else if (WSTOPSIG(status)) {
        sdebug ("child is stopped by a signal = %d\n", WSTOPSIG(status));
    }
    return 0;
}

void parent(void)
{
    wait_for_child();

    sdebug ("parent = %d\n", getpid());

    initcmdlist();

    struct argdata *arg = NULL;
    while ((arg = usercommand())) {
        do_run_cmd(arg);
        sdebug ("before freeing = %p\n", arg);
        free(arg);
        arg = NULL;
    }

}

