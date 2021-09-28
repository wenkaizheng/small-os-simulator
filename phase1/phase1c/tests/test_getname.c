#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>
static int flag = 0;

static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore.\n");
    rc = P1_V(sem);
    char name[50];
    assert(P1_SemName(0, NULL) == P1_NAME_IS_NULL);
    assert(P1_SemName(2000,name)==P1_INVALID_SID);
    assert(P1_SemName(1,name)==P1_INVALID_SID);
    assert(P1_SemName(-1,name)==P1_INVALID_SID);
    int h = P1_SemName(0, name);
    assert(h == P1_SUCCESS);
    
    USLOSS_Console("%s\n",name);

    assert(rc == P1_SUCCESS);
    // will never get here as Blocks will run and call USLOSS_Halt.
    return 12;
}

static int
Blocks(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;

    rc = P1_Fork("Unblocks", Unblocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    int tag =0;
    int cpid;
    int status;
    assert(P1GetChildStatus(tag, &cpid, &status) ==P1_SUCCESS);
    assert(status == 12);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P on semaphore.\n");
    rc = P1_P(sem);
    assert(rc == P1_SUCCESS);
    assert(flag == 1);
    char name[50];
    int h = P1_SemName(0, name);
    assert(h == P1_SUCCESS);
    
    USLOSS_Console("%s\n",name);
    USLOSS_Console("Test passed.\n");
    USLOSS_Halt(0);
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
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}