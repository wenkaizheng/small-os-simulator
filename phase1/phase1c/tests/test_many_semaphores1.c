#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

static int flags[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int sem[10];

static int
Unblocks(void *arg)
{
    int rc;
    int i = (int)arg;
    USLOSS_Console("Unblocks: Unblocks running.\n");
    flags[i] = 1;
    USLOSS_Console("Unblocks: going to v semaphore id: %d\n", i);
    rc = P1_V(sem[i]);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Unblocks: Unblocks quitting.\n");
    return 12;
}

static int
Blocks(void *arg)
{
    int rc;
    int pid0, pid1, status;
    char sname[10];
    USLOSS_Console("Blocks: Blocks running.\n");
    for (int i = 0; i < 10; i++)
    {
        rc = P1_Fork("Unblocks", Unblocks, (void *)i, USLOSS_MIN_STACK, 2, 0,
                &pid0);
        assert(rc == P1_SUCCESS);
        sprintf(sname, "%d", i);
        rc = P1_SemCreate(sname, 0, &sem[i]);
        USLOSS_Console("Blocks: create semaphore %s\n", sname);
        assert(rc == P1_SUCCESS);
        USLOSS_Console("Blocks: going to P semaphore %s id: %d\n", sname, i);
        rc = P1_P(sem[i]);
        USLOSS_Console("Blocks: semaphore %s is released\n", sname);
        assert(rc == P1_SUCCESS);
        USLOSS_Console("Blocks: going to check flags[%d]\n", i);
        assert(flags[i] == 1);
        rc = P1GetChildStatus(0, &pid1, &status);
        assert(rc == P1_SUCCESS);
        assert(pid0==pid1);
        assert(12==status);
    }
    USLOSS_Console("Blocks: freeing all.\n");
    for (int j = 0; j < 10; ++j) {
        rc = P1_SemFree(sem[j]);
        assert(rc == P1_SUCCESS);
    }
    USLOSS_Console("Blocks: Blocks quitting.\n");
    P1_Quit(11);
    // should not return
    assert(0);
    return 0;
}

void startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1SemInit();
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("Blocks", Blocks, (void *)0, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(pid == 0);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}