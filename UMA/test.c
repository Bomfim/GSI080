#include "uailloc.c"

int main()
{
<<<<<<< HEAD
	int *ptr;
	for (int i = 0; i < (2 ^ 8); i++)
	{
		ptr = (int*) malloc(i);
		printf("%p\n",&ptr);
		free(ptr);
	}
=======
	char *ptr,*ptr2,*ptr3;
	ptr = (char*) malloc(81920);
	ptr2 = (char*) malloc(40960);
	free(ptr);
	ptr3 = (char *) malloc(81920);
	
	
	fprintf(stderr, "free: %p\n", ptr);
	free(ptr2);
	fprintf(stderr, "free: %p\n", ptr2);
	free(ptr3);
	fprintf(stderr, "free: %p\n", ptr3);
	
>>>>>>> b35532aed529c3717ea935ae0f2b6582191e68cc
	return 0;
}
