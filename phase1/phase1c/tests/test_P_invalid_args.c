/*
 * Tests calling P1_P() with invalid sids 
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include "tester.h"

static int
Worker(void *arg)
{   
    int rc;
    
    USLOSS_Console("Worker: Running.\n");

    USLOSS_Console("Worker: Calling P(-1)\n");
    rc = P1_P(-1);
    TEST(rc, P1_INVALID_SID);
    PASSED();
    // should not return
    assert(0);
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc;
    int sem;
    int pid;

    P1SemInit();
    USLOSS_Console("startup: Creating 1 semaphore with initial value 0\n");
    rc = P1_SemCreate("sem0", 0, &sem);
    TEST(rc, P1_SUCCESS);

    USLOSS_Console("startup: Forking a process Worker\n");
    rc = P1_Fork("Worker", Worker, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
