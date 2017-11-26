#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include "fiber.h"

sigset_t set;                         /* process wide signal mask */
ucontext_t signal_context;            /* the interrupt context */
void *signal_stack;                   /* global interrupt stack */
fiber_t fiberList[MAX_FIBERS];        /* Queue of fibers*/
ucontext_t cur_context, main_context; /* a pointer to the current_context */
static int numFibers = 0;             /* number of active fibers */
static int index;

/* helper function to create a context.
initialize the context from the current context, setup the new
stack, signal mask, and tell it which function to call.
*/
void mkcontext(ucontext_t *uc, void *function)
{
    void *stack;
    getcontext(uc);
    stack = malloc(FIBER_STACK);
    if (stack == NULL)
    {
        perror("malloc");
        exit(1);
    }

    /* we need to initialize the ucontext structure, give it a stack,
    flags, and a sigmask */
    uc->uc_stack.ss_sp = stack;
    uc->uc_stack.ss_size = FIBER_STACK;
    uc->uc_stack.ss_flags = 0;
    if (sigemptyset(&uc->uc_sigmask) < 0)
    {
        perror("sigemptyset");
        exit(1);
    }

    /* setup the function we're going to, and n-1 arguments. */
    makecontext(uc, function, 0);
    fiberList[numFibers].active = 1;
    printf("context is %p\n", uc);
}

int isActiveFibers(int *index)
{
    for (int i = 0; i < sizeof(fiberList) / sizeof(fiberList[0]); i++)
    {
        if (fiberList[i].active == 1)
        {
            if (index != NULL)
                *index = i;
            return 1;
        }
    }
    return 0;
}

void scheduler() //signal handler for SIGPROF
{
    printf("Scheduler! - signal_context:%p\n", signal_context);
    while (isActiveFibers(&index))
    {
        setcontext(&cur_context);
        cur_context = fiberList[index].context;
    }
    // setcontext(&main_context);
}

/*
Timer interrupt handler.
Creates a new context to run the scheduler in, masks signals, then swaps
contexts saving the previously executing thread and jumping to the
scheduler.
*/
void timer_interrupt(int j, siginfo_t *si, void *old_context)
{
    /* Create new scheduler context */
    getcontext(&signal_context);
    signal_context.uc_stack.ss_sp = signal_stack;
    signal_context.uc_stack.ss_size = FIBER_STACK;
    signal_context.uc_stack.ss_flags = 0;
    sigemptyset(&signal_context.uc_sigmask);
    makecontext(&signal_context, scheduler, 0);

    /* save running thread, jump to scheduler */
    if (&cur_context)
        swapcontext(&cur_context, &signal_context);
}

/* Set up SIGALRM signal handler */
void setup_signals(void)
{
    struct sigaction act;
    act.sa_sigaction = timer_interrupt;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    if (sigaction(SIGALRM, &act, NULL) != 0)
    {
        perror("Signal handler");
    }
}

void initialize()
{
    struct itimerval it;
    fprintf(stderr, "Process Id: %d\n", (int)getpid());
    /* allocate the global signal/interrupt stack */
    signal_stack = malloc(FIBER_STACK);
    if (signal_stack == NULL)
    {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < MAX_FIBERS; ++i)
    {
        fiberList[i].active = 0;
    }

    /* initialize the signal handlers */
    setup_signals();

    /* setup our timer */
    it.it_interval.tv_sec = 1;
    it.it_interval.tv_usec = 0;
    it.it_value = it.it_interval;
    if (setitimer(ITIMER_REAL, &it, NULL))
        perror("setitimer");
}

int fiber_create(fiber_t *fiber, void *(*start_routine)(void *), void *arg)
{
    fiberList[numFibers] = *fiber;

    if (numFibers == MAX_FIBERS)
        return -1;

    if (numFibers == 0)
        initialize();

    mkcontext(&fiberList[numFibers].context, start_routine);
    cur_context = fiberList[numFibers].context;
    numFibers++;
    swapcontext(&main_context, &cur_context);

    return 0;
}

int fiber_join(fiber_t fiber, void **retval)
{
    printf("Into Join\n");
    while (fiber.active == 1)
        setcontext(&signal_context);

    return 0;
}

void fiber_exit(void *retval)
{
    fiberList[index].active = 0;
    numFibers--;
    printf("index:%d\nnumFilbers:%d\nInto Exit\n", index, numFibers);
    if(isActiveFibers(NULL))
        setcontext(&signal_context);
    else
        setcontext(&main_context);
}
