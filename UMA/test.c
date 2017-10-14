#include "uailloc.c"

int main()
{
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
	
	return 0;
}
