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

static int sem;
static int count;

static int 
Worker(void *arg)
{
    int rc;

    // Wait on the semaphore.
    rc = P1_P(sem);
    TEST(rc, P1_SUCCESS);
    // Increment the count. Depends on dispatcher being run-to-completion.
    count++;
    return 0;
}

/*
 * Controller process that runs the show. Create the workers, V one at a time, make sure
 * count only increments by 1.
 */

static int 
Controller(void *arg)
{
    int rc;
    int pid;
    int i;

    for (i = 0; i < NUMPROCS; i++) {  
        rc = P1_Fork(MakeName("Worker", i), Worker, (void *) i, USLOSS_MIN_STACK, 1, 0, &pid);
        assert(rc == P1_SUCCESS);
    }
    for (i = 0; i < NUMPROCS; i++) {
        int before = count;
        rc = P1_V(sem);
        TEST(rc, P1_SUCCESS);
        // We are lower priority than the workers, so if multiple of them woke up the
        // count will be more than before+1.
        TEST(before+1, count)
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
    rc = P1_SemCreate("Sem", 0, &sem);
    TEST(rc, P1_SUCCESS);

    rc = P1_Fork("Controller", Controller, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
