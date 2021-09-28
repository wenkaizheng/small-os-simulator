/*
 * V a higher priority process. The V'ed process should run immediately.
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tester.h"


static int sem;
static int running;
static int flag = 0;

static int 
Blocker(void *arg)
{
    int rc;

    // Let our parent know we are running.
    rc = P1_V(running);
    TEST(rc, P1_SUCCESS);
    // Wait to proceed. We should return from this after the Unblocker quits.
    rc = P1_P(sem);
    TEST(rc, P1_SUCCESS);
    flag = 1;
    return 0;
}

static int
Unblocker(void *arg)
{
    int rc;

    // V the Blocker. We should continue to run.
    rc = P1_V(sem);
    TEST(rc, P1_SUCCESS);
    flag = 2;
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

    rc = P1_Fork("Blocker", Blocker, NULL, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    // Wait for the Blocker to run and P its semaphore.
    rc = P1_P(running);
    TEST(rc, P1_SUCCESS);
    // Create an Unblocker that is lower priority than the Blocker.
    rc = P1_Fork("Unblocker", Unblocker, NULL, USLOSS_MIN_STACK, 3, 0, &pid);
    assert(rc == P1_SUCCESS);
    // We only run if all other processes quit. 
    TEST(flag, 2);
    PASSED();
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc;
    int pid;

    P1SemInit();
    rc = P1_SemCreate("sem", 0, &sem);
    TEST(rc, P1_SUCCESS);
    rc = P1_SemCreate("running", 0, &running);
    TEST(rc, P1_SUCCESS);

    rc = P1_Fork("Controller", Controller, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
