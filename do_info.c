#include "debug.h"

extern struct cmdlist *commands ;

int handle_info(struct argdata *arg)
{
    if (NULL == arg->v[1]) {
        debug ("info What?\n");
        return 1;
    }

    struct cmdlist *tmp = match_cmd(NULL, arg->v[1]);

    if (NULL == tmp) {
        debug("no help available for %10s\n", arg->v[1]);
        return 1;
    }

    if (NULL == tmp->help) {
        debug("no help available for %10s\n", arg->v[1]);
        return 1;
    }
        return tmp->help();

}

