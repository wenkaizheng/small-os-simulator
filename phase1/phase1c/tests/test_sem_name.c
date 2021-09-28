#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "tester.h"

static int
SemNameCheck(void *arg) 
{
    int sem = (int) arg;
    int rc;

    char name[P1_MAXNAME+1];

    rc = P1_SemName(sem, name);
    TEST(rc, P1_SUCCESS);
    PASSED();
    // should not return
    assert(0);
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    int sem;

    P1SemInit();
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);

    // Checks Semaphore name
    rc = P1_Fork("SemNameCheck", SemNameCheck, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}