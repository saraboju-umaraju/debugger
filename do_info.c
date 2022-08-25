#include "debug.h"

extern struct cmdlist *commands ;

int handle_info(void *data)
{
    struct argdata *arg = (struct argdata*)data;
    sdebug ("%s %s %s \n", arg->v0, arg->v1, arg->v2);

    if (!arg->v1[0]) {
        debug ("info would need an argument\n");
        return 1;
    }

    struct cmdlist *iter = commands;
    for(;iter;) {
        if (cmd_match(iter, arg->v1)) {
            if (iter->help) {
                iter->help();
                return 0;
            }
        }
        iter = iter->next;
    }
    debug("no help available for %10s\n", arg->v1);
    return 1;
}

