/*
 * Create multiple processes that wait on a single semaphore. Make sure only one wakes up
 * when the semaphore is V'ed.
 */

#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"

#define NUMPROCS 10

static int sems[2];
static int flag = 0;

static int 
Worker(void *arg)
{
    int rc;

    // Grab the first semaphore.
    rc = Sys_SemP(sems[0]);
    assert(rc== P1_SUCCESS);

    // Wait on the second semaphore.
    rc = Sys_SemP(sems[1]);
    assert(rc== P1_SUCCESS);
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

    rc = Sys_Spawn("Worker", Worker, NULL, USLOSS_MIN_STACK, 1,&pid);
    assert(rc == P1_SUCCESS);

    // V the first semaphore. Worker should still be blocked.
    rc = Sys_SemV(sems[0]);
    assert(rc== P1_SUCCESS);
    assert(flag== 0);

    // V the second semaphore. Worker should still unblock.
    rc = Sys_SemV(sems[1]);
    assert(rc== P1_SUCCESS);
    assert(flag== 1);

   // PASSED();
    return 0;
}

int P3_Startup(void *arg) 
{
    int rc;
    int pid;

    //P1SemInit();
    rc = Sys_SemCreate(MakeName("sem", 0), 1, &sems[0]);
    assert(rc== P1_SUCCESS);

    rc = Sys_SemCreate(MakeName("sem", 1), 0, &sems[1]);
    assert(rc== P1_SUCCESS);

    rc = Sys_Spawn("Controller", Controller, NULL, USLOSS_MIN_STACK, 5, &pid);
    assert(rc == P1_SUCCESS);    
    return 0;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}
