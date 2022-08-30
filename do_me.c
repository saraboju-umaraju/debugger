#include <stdio.h>

int call_function()
{
    printf ("Just another function\n");
    int a = 9;
    int b = 8;
    int c;
    c = a + b;
    return c;
}

int main (int argc, char *argv[])
{
    printf ("Hello, I am ALIVE ;)\n");
    printf( "I am past the first break point possibly \n");
    call_function();
	return 0;
}

