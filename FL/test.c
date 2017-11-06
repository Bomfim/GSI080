#include <stdio.h>
#include "fiber.h"

void * function(){
    printf("AEHOOOOOOOOOOOO!\n");
}

int main(void)
{
    fiber_t fiber;

    if(fiber_create(&fiber,function,NULL));



    return 0;
}