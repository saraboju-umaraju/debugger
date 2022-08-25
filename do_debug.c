#include "debug.h"

pid_t proc = -1;

int main (int argc, char *argv[])
{

    proc = fork();
    switch (proc) {
        case 0:
            child();
            break;
        case -1:
            sdebug ("looks like fork has failed\n");
            exit(EXIT_FAILURE);
            break;
        default:
            parent();
            break;
    }
	return 0;
}

