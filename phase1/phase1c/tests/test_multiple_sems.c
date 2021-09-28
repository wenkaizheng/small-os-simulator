/*
 * Create multiple processes that wait on individual semaphores. Make sure the proper
 * process wakes up when its semaphore is V'ed.
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tester.h"

#define NUMPROCS 10

static int sems[P1_MAXPROC];
static int pids[P1_MAXPROC];

static int 
Worker(void *arg)
{
    int i = (int) arg;
    int rc;

    // Wait on our semaphore.
    rc = P1_P(sems[i]);
    TEST(rc, P1_SUCCESS);
    return P1_GetPid();
}

/*
 * Controller process that runs the show. Create the workers, V them randomly, and make
 * sure the correct worker quits each time.
 */

static int 
Controller(void *arg)
{
    int rc;
    int pid;
    int status;
    int i;

    for (i = 0; i < NUMPROCS; i++) {  
        rc = P1_Fork(MakeName("Worker", i), Worker, (void *) i, USLOSS_MIN_STACK, 3, 0, &pids[i]);
        assert(rc == P1_SUCCESS);
    }
    int count = NUMPROCS;
    while(count > 0) {
        i = random() % count;
        rc = P1_V(sems[i]);
        TEST(rc, P1_SUCCESS);
        rc = P1GetChildStatus(0, &pid, &status);
        assert(rc == P1_SUCCESS);
        TEST(pid, pids[i]);
        TEST(status, pid);
        pids[i] = pids[count-1];
        sems[i] = sems[count-1];
        count--;
    }
    PASSED();
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc;
    int pid;

    P1SemInit();
    for (int i = 0; i < NUMPROCS; i++) {
        rc = P1_SemCreate(MakeName("Sem", i), 0, &sems[i]);
        TEST(rc, P1_SUCCESS);
    }
    rc = P1_Fork("Controller", Controller, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
