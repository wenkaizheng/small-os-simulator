#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "tester.h"

static int flag = 0;

static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore with invalid sem id.\n");
    rc = P1_V(sem);
    TEST(rc, P1_INVALID_SID);
    return 12;
}

static int
Blocks(void *arg) 
{
    int rc;
    int pid;

    rc = P1_Fork("Unblocks", Unblocks, (void *) -1, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore.\n");
    // rc = P1_P(sem);
    // assert(rc == P1_SUCCESS);
    assert(flag == 1);
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
    TEST(rc, P1_SUCCESS);
    // Unblocks V's semaphore before Block P's
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}