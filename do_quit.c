#include "debug.h"

int handle_quit(struct argdata *data)
{
    free_vector(data);
    free(data);
    data = NULL;
    exit(0);
    return 0;
}


