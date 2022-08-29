#include <stdio.h>

int call_function()
{
    printf ("Just another function\n");
    return 0;
}

int main (int argc, char *argv[])
{
    printf ("Hello, I am ALIVE ;)\n");
    printf( "I am past the first break point possibly \n");
    call_function();
	return 0;
}

