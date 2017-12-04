#ifndef FIBERLIB_H

#define MAX_FIBERS 9

#define FIBER_STACK (1024 * 64)

#define INTERVAL 100

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KBLU  "\x1B[34m"
#define RESET "\033[0m"

#include <ucontext.h>

typedef struct
{
	int fiberID;
	ucontext_t context;
	int active;
} fiber_t;


int fiber_create(fiber_t *fiber, void *(*start_routine)(void *), void *arg);

int fiber_join(fiber_t fiber, void **retval);

void fiber_exit(void *retval);
#endif
