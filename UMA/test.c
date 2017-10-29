#include <stdio.h>
#include "uailloc.c"
int main()
{
	int *ptr;
	for (int i = 0; i < (2 ^ 8); i++)
	{
		ptr = (int*) malloc(i);
		printf("%p\n",&ptr);
		free(ptr);
	}
	return 0;
}
