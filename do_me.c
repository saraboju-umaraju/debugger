#include <stdio.h>

extern int call_function();

int main (int argc, char *argv[])
{
    printf ("Hello, I am ALIVE ;)\n");
    printf( "I am past the first break point possibly \n");
    call_function();
	return 0;
}

