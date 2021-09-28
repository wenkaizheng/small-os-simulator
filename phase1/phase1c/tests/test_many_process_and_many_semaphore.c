#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

static int flags[4] = {0, 0, 0, 0};
static int sems[4];


static int
UnblocksAll(void *arg) {
    int rc;
    int id = (int) arg;
    USLOSS_Console("Unblock %d: Unblocks running.\n", id);
    for (int i = 0; i < 100; ++i) {
        USLOSS_Console("Unblock %d: going to V semaphore\n", id);
        rc = P1_V(sems[1]);
        assert(rc == P1_SUCCESS);
        rc = P1_V(sems[2]);
        assert(rc == P1_SUCCESS);
        rc = P1_V(sems[3]);
        assert(rc == P1_SUCCESS);
    }
    USLOSS_Console("Unblock %d: Unblocks quitting.\n", id);
    return 0;
}


static int
Unblocks(void *arg) {
    int rc;
    int id = (int) arg;
    int semId = id % 4;
    USLOSS_Console("Unblock %d: Unblocks running.\n", id);
    for (int i = 0; i < 100; ++i) {
        USLOSS_Console("Unblock %d: going to V semaphore\n", id);
        rc = P1_V(sems[semId]);
        assert(rc == P1_SUCCESS);
    }
    USLOSS_Console("Unblock %d: Unblocks quitting.\n", id);
    return 0;
}


static int
Blocks(void *arg) {
    int rc;
    int id = (int) arg;
    int semId = id % 4;
    USLOSS_Console("Block %d: Blocks running.\n", id);
    for (int i = 0; i < 10; ++i) {
        USLOSS_Console("Block %d: going to P semaphore\n", id);
        rc = P1_P(sems[semId]);
        USLOSS_Console("Block %d: semaphore is released\n", id);
        assert(rc == P1_SUCCESS);
        USLOSS_Console("(%d * 10) + %d/4 = 0+%d == %d\n", i,id,id/4,
                flags[semId]);
        assert((i * 10) + id/4 == flags[semId]);
        flags[semId]++;
    }
    USLOSS_Console("Block %d: Blocks quitting.\n", id);


    P1_Quit(0);
    // should not return
    assert(0);
    return 0;
}


static int helper(void *arg) {
    int rc;
    int pid_in[100], pid_out[100], status;
    int expected_status = 0;
    char sname1[20] = "UA beat UCLA0",
            sname2[20] = "UA beat UCLA1",
            sname3[20] = "UA beat UCLA2",
            sname4[20] = "UA beat UCLA3",
            name[20];
    USLOSS_Console("HELPER: going to create semaphores\n");
    rc = P1_SemCreate(sname1, 0, &(sems[0]));
    assert(rc == P1_SUCCESS);

    USLOSS_Console("HELPER: created %s\n", sname1);
    rc = P1_SemCreate(sname2, 0, &(sems[1]));
    assert(rc == P1_SUCCESS);
    USLOSS_Console("HELPER: created %s\n", sname2);
    rc = P1_SemCreate(sname3, 0, &(sems[2]));
    assert(rc == P1_SUCCESS);
    USLOSS_Console("HELPER: created %s\n", sname3);
    rc = P1_SemCreate(sname4, 0, &(sems[3]));
    assert(rc == P1_SUCCESS);
    USLOSS_Console("HELPER: created %s\n", sname4);

    int i=0;

    for (i  = 0; i <40 ; i++) {
        sprintf(name, "Blocks%d", i);
        USLOSS_Console("HELPER: going to create %s\n", name);
        rc = P1_Fork(name, Blocks, (void *) i, USLOSS_MIN_STACK, 1, 0,
                     &(pid_in[i]));
        assert(rc == P1_SUCCESS);
    }

    rc = P1_Fork("Unblocks1", Unblocks, (void *) 0, USLOSS_MIN_STACK,
                 4, 0, &(pid_in[40]));
    assert(rc == P1_SUCCESS);
    rc = P1_SemFree(sems[0]);
    assert(rc == P1_SUCCESS);


    rc = P1_Fork("UnblocksAll", UnblocksAll, (void *) 123, USLOSS_MIN_STACK,
                4, 0, &(pid_in[41]));
    assert(rc == P1_SUCCESS);
    rc = P1_SemFree(sems[1]);
    assert(rc == P1_SUCCESS);
    rc = P1_SemFree(sems[2]);
    assert(rc == P1_SUCCESS);
    rc = P1_SemFree(sems[3]);
    assert(rc == P1_SUCCESS);



    int j;
    for (j = 0; j < 42; ++j) {
        USLOSS_Console("HELPER: going to get child %d status\n", j);
        rc = P1GetChildStatus(0, &(pid_out[j]), &status);
        assert(rc == P1_SUCCESS);
        assert(pid_in[j] == j+1);
        assert(status == 0);
    }
    USLOSS_Console("pass all\n");
    return 0;
}

void func(int dev, void *arg){

}


void startup(int argc, char **argv) {
    int pid;
    int rc;
    P1SemInit();
    USLOSS_IntVec[USLOSS_CLOCK_INT] =  func;
    // Blocks blocks then Unblocks unblocks it
    rc = P1_Fork("helper", helper, (void *) 0, USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(pid == 0);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}