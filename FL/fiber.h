typedef struct fiber_t{
    
}fiber_t;

int fiber_create(fiber_t *fiber, void *(*start_routine) (void *), void *arg);

int fiber_join(fiber_t fiber, void **retval);

void fiber_exit(void *retval);