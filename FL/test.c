#include <stdio.h>
#include "fiber.h"
#include <unistd.h>

void *function()
{
    for (;;)
    {
        printf("AEHOOOOOOOOOOOO!\n");
        sleep(1);
    }
}

void *fibonacci()
{
    int i;
    int fib[2] = {0, 1};

    printf("fibonacci(0) = 0\nfibonnaci(1) = 1\n");
    for (i = 2; i < 15; ++i)
    {
        int nextFib = fib[0] + fib[1];
        printf("fibonacci(%d) = %d\n", i, nextFib);
        fib[0] = fib[1];
        fib[1] = nextFib;
    }
}

void *squares()
{
    int i;

    for (i = 0; i < 10; ++i)
    {
        printf("%d*%d = %d\n", i, i, i * i);
    }
}
int main(void)
{
    fiber_t fiber[3];

    fiber_create(&fiber[0], function, NULL);
    fiber_create(&fiber[1], fibonacci, NULL);
    fiber_create(&fiber[2], squares, NULL);

    return 0;
}