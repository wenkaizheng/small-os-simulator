/*
 * Create multiple processes that wait on a single semaphore. Make sure only one wakes up
 * when the semaphore is V'ed.
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tester.h"

#define NUMPROCS 10

static int sems[2];
static int flag = 0;

static int 
Worker(void *arg)
{
    int rc;

    // Grab the first semaphore.
    rc = P1_P(sems[0]);
    TEST(rc, P1_SUCCESS);

    // Wait on the second semaphore.
    rc = P1_P(sems[1]);
    TEST(rc, P1_SUCCESS);
    flag = 1;
    return 0;
}

/*
 * Controller process that runs the show. Create the worker, V the first semaphore which should
 * do nothing, then V the second semaphore to allow the worker to finish.
 */

static int 
Controller(void *arg)
{
    int rc;
    int pid;

    rc = P1_Fork("Worker", Worker, NULL, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);

    // V the first semaphore. Worker should still be blocked.
    rc = P1_V(sems[0]);
    TEST(rc, P1_SUCCESS);
    TEST(flag, 0);

    // V the second semaphore. Worker should still unblock.
    rc = P1_V(sems[1]);
    TEST(rc, P1_SUCCESS);
    TEST(flag, 1);

    PASSED();
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc;
    int pid;

    P1SemInit();
    rc = P1_SemCreate(MakeName("sem", 0), 1, &sems[0]);
    TEST(rc, P1_SUCCESS);

    rc = P1_SemCreate(MakeName("sem", 1), 0, &sems[1]);
    TEST(rc, P1_SUCCESS);

    rc = P1_Fork("Controller", Controller, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
