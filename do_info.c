#include "debug.h"

extern struct cmdlist *commands ;

int handle_info(void *data)
{
    struct argdata *arg = (struct argdata*)data;

    if (!arg->v1[0]) {
        debug ("info needs arg\n");
        return 1;
    }

    struct cmdlist *tmp = match_cmd(data, arg->v1);

    if (NULL == tmp) {
        debug("no help available for %10s\n", arg->v1);
        return 1;
    }

    if (NULL == tmp->help) {
        debug("no help available for %10s\n", arg->v0);
        return 1;
    }
        return tmp->help();

}

