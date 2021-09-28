/*
 * V a higher priority process. The V'ed process should run immediately.
 */

#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"


static int sem;
static int running;
static int flag = 0;

static int 
Blocker(void *arg)
{
    int rc;

    // Let our parent know we are running.
    rc = Sys_SemV(running);
    assert(rc== P1_SUCCESS);
    // Wait to proceed. We should return from this after the Unblocker quits.
    rc = Sys_SemP(sem);
    assert(rc== P1_SUCCESS);
    flag = 1;
    USLOSS_Console("Blocker done in here\n");
    return 0;
}

static int
Unblocker(void *arg)
{
    int rc;

    // V the Blocker. We should continue to run.
    rc = Sys_SemV(sem);
    assert(rc== P1_SUCCESS);
    flag = 2;
    USLOSS_Console("Unblocker done in here\n");
    return 0;
}


/*
 * Controller process that runs the show. Creates Blocker at priority 2 and once it
 * runs and blocks creates an Unblocker at priority 3. The Unblocker should not run 
 * after V'ing the Blocker so the flag should be 2.
 */

static int 
Controller(void *arg)
{
    int rc;
    int pid;

    rc = Sys_Spawn("Blocker", Blocker, NULL, USLOSS_MIN_STACK, 2, &pid);
    assert(rc == P1_SUCCESS);
    // Wait for the Blocker to run and P its semaphore.
    rc = Sys_SemP(running);
    assert(rc== P1_SUCCESS);
    // Create an Unblocker that is lower priority than the Blocker.
    rc = Sys_Spawn("Unblocker", Unblocker, NULL, USLOSS_MIN_STACK, 3, &pid);
    assert(rc == P1_SUCCESS);
    // We only run if all other processes quit. 
    assert(flag== 2);
    USLOSS_Console("Controller done in here\n");
    //PASSED();
    return 0;
}

int P3_Startup(void *arg) 
{
    int rc;
    int pid;

   // P1SemInit();
    rc = Sys_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    rc = Sys_SemCreate("running", 0, &running);
    assert(rc ==P1_SUCCESS);

    rc = Sys_Spawn("Controller", Controller, NULL, USLOSS_MIN_STACK, 5, &pid);
    assert(rc == P1_SUCCESS);  
    USLOSS_Console("P3 done in here\n");  
    return 0;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}
