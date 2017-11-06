#include <ucontext.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <semaphore.h>
#include "fiber.h"

#define NUMCONTEXTS 8
#define STACKSIZE 64 * 1024
#define INTERVAL 100

sigset_t set;              /* process wide signal mask */
ucontext_t signal_context; /* the interrupt context */
void *signal_stack;        /* global interrupt stack */
ucontext_t contexts[NUMCONTEXTS];
int curcontext = 0;
ucontext_t *cur_context; /* a pointer to the current_context */
int vet[8][1];
sem_t semaphore;

void *func()
{
}

/* helper function to create a context.
initialize the context from the current context, setup the new
stack, signal mask, and tell it which function to call.
*/
void mkcontext(ucontext_t *uc, void *function)
{
    void *stack;
    getcontext(uc);
    stack = malloc(STACKSIZE);
    if (stack == NULL)
    {
        perror("malloc");
        exit(1);
    }

    /* we need to initialize the ucontext structure, give it a stack,
	flags, and a sigmask */
    uc->uc_stack.ss_sp = stack;
    uc->uc_stack.ss_size = STACKSIZE;
    uc->uc_stack.ss_flags = 0;
    if (sigemptyset(&uc->uc_sigmask) < 0)
    {
        perror("sigemptyset");
        exit(1);
    }

    /* setup the function we're going to, and n-1 arguments. */
    makecontext(uc, function, 1);
    printf("context is %p\n", uc);
}

void scheduler() //signal handler for SIGPROF
{
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
    signal_context.uc_stack.ss_size = STACKSIZE;
    signal_context.uc_stack.ss_flags = 0;
    sigemptyset(&signal_context.uc_sigmask);
    makecontext(&signal_context, scheduler, 1);

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
    int i;
    struct itimerval it;
    fprintf(stderr, "Process Id: %d\n", (int)getpid());
    /* allocate the global signal/interrupt stack */
    signal_stack = malloc(STACKSIZE);
    if (signal_stack == NULL)
    {
        perror("malloc");
        exit(1);
    }

    memset(vet, 0, 8 * 1 * sizeof(int));

    /* make all our contexts */
    mkcontext(&contexts[0], func);
    for (i = 1; i < NUMCONTEXTS; i++)
    {
        mkcontext(&contexts[i], func);
    }

    /* initialize the signal handlers */
    setup_signals();

    /* setup our timer */
    it.it_interval.tv_sec = 1;
    it.it_interval.tv_usec = 0;
    it.it_value = it.it_interval;
    if (setitimer(ITIMER_REAL, &it, NULL))
        perror("setitimer");

    /* force a swap to the first context */
    cur_context = &contexts[0];
    setcontext(&contexts[0]);
}

int fiber_create(fiber_t *fiber, void *(*start_routine)(void *), void *arg)
{
    initialize();
}

int fiber_join(fiber_t fiber, void **retval)
{
}

void fiber_exit(void *retval)
{
}
