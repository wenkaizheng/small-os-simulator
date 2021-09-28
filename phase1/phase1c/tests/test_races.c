/*
 * Tests for race conditions in P and V.
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tester.h"

#define NUM_ITERS 100000
#define NUM_PROCS 10

static int mutex;
static volatile int count;

static int 
Proc(void *arg)
{
    int num = (int) arg;
    int rc;

    USLOSS_Console("Proc%d: Running.\n", num);
    for (int i = 0; i < NUM_ITERS; i++) {
        rc = P1_P(mutex);
        TEST(rc, P1_SUCCESS);

        count++;

        rc = P1_V(mutex);
        TEST(rc, P1_SUCCESS);
    }
    USLOSS_Console("Proc%d: Work done. Quitting.\n", num);
    return 12;
}

// Process that creates the workers. Runs at the highest priority so all the workers are
// created before any of them start.
static int
Starter(void *arg)
{
    int rc;
    int pid;

    // Create the worker processes.
    for (int i = 0; i < NUM_PROCS; i++) {
        rc = P1_Fork(MakeName("Proc", i), Proc, (void *) i, USLOSS_MIN_STACK, 2, 0, &pid);
        assert(rc == P1_SUCCESS);
    }
    // Our job is done. Once we quit the workers will start running.
    return 0;
}


// Controller process that runs the show. Creates a higher priority Starter that creates
// the workers, then waits for the workers to finish.

static int 
Controller(void *arg)
{
    int rc;
    int pid;

    rc = P1_Fork("Starter", Starter, NULL, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    TEST(count, NUM_PROCS * NUM_ITERS);
    PASSED();
    return 0;
}


/*
 * Force a context switch on every interrupt.
 */

static void
ClockHandler(int type, void *arg) 
{
    USLOSS_Console("tick\n");
    P1Dispatch(TRUE);
}


void
startup(int argc, char **argv)
{
    int rc;
    int pid;

    P1SemInit();
    USLOSS_IntVec[USLOSS_CLOCK_INT] = ClockHandler;
    USLOSS_Console("startup: Creating mutex semaphore with initial value 1\n");
    rc = P1_SemCreate("sem0", 1, &mutex);
    TEST(rc, P1_SUCCESS);

    rc = P1_Fork("Controller", Controller, NULL, USLOSS_MIN_STACK, 3, 0, &pid);
    assert(rc == P1_SUCCESS);    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
