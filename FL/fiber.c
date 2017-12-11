#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "fiber.h"

sigset_t set;                                           /* process wide signal mask */
void *signal_stack;                                     /* global interrupt stack */
fiber_t fiberList[MAX_FIBERS];                          /* Queue of fibers*/
ucontext_t *cur_context, *main_context, signal_context; /* a pointer to the current_context */
static int numFibers = 0;                               /* number of active fibers */

/* helper function to create a context.
initialize the context from the current context, setup the new
stack, signal mask, and tell it which function to call.
*/
int mkcontext(ucontext_t *uc, void *function, void *arg)
{
    void *stack;
    getcontext(uc);
    stack = malloc(FIBER_STACK);
    if (stack == NULL)
    {
        perror("Não foi possível concluir a alocação");
        return -1;
    }

    /* we need to initialize the ucontext structure, give it a stack,
    flags, and a sigmask */
    uc->uc_stack.ss_sp = stack;
    uc->uc_stack.ss_size = FIBER_STACK;
    uc->uc_stack.ss_flags = 0;
    if (sigemptyset(&uc->uc_sigmask) < 0)
    {
        perror("sigemptyset");
        return -2;
    }

    /* setup the function we're going to, and n-1 arguments. */
    makecontext(uc, function, 1, arg);
    fiberList[numFibers].active = 1;
    // printf("\ncontext is %p\n\n", uc);
    return 1;
}

static void scheduler() //signal handler for SIGPROF
{
    // printf("Scheduler!\n");
    int i = 0, j;
    if (cur_context != &fiberList[0].context)
    {
        for (i = 1; i < 9; i++)
        {
            if (cur_context == &fiberList[i].context)
            {
                fiberList[i].context = *cur_context;
                break;
            }
        }
    }
    for (j = i + 1; j < 9; j++)
    {
        if (fiberList[j].active)
        {
            cur_context = &fiberList[j].context;
            setcontext(cur_context);
        }
    }

    if (fiberList[0].active == 1)
    {
        cur_context = &fiberList[0].context;
        setcontext(&fiberList[0].context);
    }
    else
    {
        for (j = 1; j < 9; j++)
        {
            if (fiberList[j].active)
            {
                cur_context = &fiberList[j].context;
                setcontext(cur_context);
            }
        }
    }
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
    swapcontext(cur_context, &signal_context);
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
    // fprintf(stderr, "Process Id: %d\n", (int)getpid());
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
        fiberList[i].fiberID = i;
    }

    /* initialize the signal handlers */
    setup_signals();

    /* setup our timer */
    it.it_interval.tv_sec = 1;
    it.it_interval.tv_usec = 0;
    it.it_value = it.it_interval;
    if (setitimer(ITIMER_REAL, &it, NULL))
        perror("setitimer");
    fiberList[0].active = 1;
}

int fiber_create(fiber_t *fiber, void *(*start_routine)(void *), void *arg)
{
    if (numFibers == MAX_FIBERS)
        return -1;

    if (numFibers == 0)
        initialize();

    numFibers++;
    fiberList[numFibers] = *fiber;
    if(mkcontext(&fiberList[numFibers].context, start_routine, arg))
        cur_context = &fiberList[numFibers].context;
    swapcontext(&fiberList[0].context, cur_context);

    return 1;
}

int fiber_join(fiber_t fiber, void **retval)
{
    // printf("Into Join\n");
    fiberList[0].active = 0;
    // swapcontext(&fiberList[0].context, &signal_context);
    scheduler();
    return 1;
}

void fiber_exit(void *retval)
{
    int i = 1;
    // printf("Into Exit\n");
    fiberList[0].active = 1;
    numFibers--;
    while (cur_context != &fiberList[i].context)
        i++;
    fiberList[i].active = 0;
    // swapcontext(cur_context, &signal_context);
    scheduler();
}
