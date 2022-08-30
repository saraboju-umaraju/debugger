#include "debug.h"
#if defined(ALLOC_DEBUG)

void *__real_malloc(size_t n);
void __real_free(void *ptr);

void *__wrap_malloc(size_t n)
{
    printf ("Allocating %zu bytes\n", n);
    return __real_malloc(n);
}

void __wrap_free(void *ptr)
{
    printf (" __UMA__ %s %s %d\n",__FILE__,__func__,__LINE__);
    __real_free(ptr);
}
#endif

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

