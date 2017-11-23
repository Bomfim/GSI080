#ifndef FIBERLIB_H

#define MAX_FIBERS 8

#define FIBER_STACK (1024 * 64)

#define INTERVAL 100

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
