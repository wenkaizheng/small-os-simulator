#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

static int flag = 0;
static int sem;


static int
Unblocks(void *arg) {
    int rc;
    char *name = (char *) arg;
    USLOSS_Console("%s: Unblocks running.\n", name);
    flag += 1;
    USLOSS_Console("%s: going to V semaphore\n", name);
    rc = P1_V(sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("%s: Unblocks quitting.\n", name);
    return *(name + 8) - '0';
}

static int
Blocks(void *arg) {
    int rc;
    char *name = (char *) arg;
    USLOSS_Console("%s: Blocks running.\n", name);
    USLOSS_Console("%s: going to P semaphore\n", name);
    rc = P1_P(sem);
    USLOSS_Console("%s: semaphore is released\n", name);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("%s: Blocks quitting.\n", name);
    P1_Quit(*(name + 6) - '0');
    // should not return
    assert(0);
    return 0;
}


static int helper(void *arg) {
    int rc;
    int pid_in[15], pid_out[15], status;
    int expected_status[15] = {1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 5, 7, 0, 0, 0};
    char sname[20] = "UA beat UCLA";
    rc = P1_SemCreate(sname, 0, &sem);
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Blocks1", Blocks, (void *) "Blocks1", USLOSS_MIN_STACK, 1, 0,
                 &(pid_in[1]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Blocks2", Blocks, (void *) "Blocks2", USLOSS_MIN_STACK, 1, 0,
                 &(pid_in[3]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Blocks3", Blocks, (void *) "Blocks3", USLOSS_MIN_STACK, 1, 0,
                 &(pid_in[5]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Blocks4", Blocks, (void *) "Blocks4", USLOSS_MIN_STACK, 1, 0,
                 &(pid_in[7]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks1", Unblocks, (void *) "Unblocks1", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[0]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks2", Unblocks, (void *) "Unblocks2", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[2]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks3", Unblocks, (void *) "Unblocks3", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[4]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks4", Unblocks, (void *) "Unblocks4", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[6]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks5", Unblocks, (void *) "Unblocks5", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[8]));
    assert(rc == P1_SUCCESS);
    USLOSS_Console("%d", sem);
    rc = P1_Fork("Unblocks6", Unblocks, (void *) "Unblocks6", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[9]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Blocks5", Blocks, (void *) "Blocks5", USLOSS_MIN_STACK, 1, 0,
                 &(pid_in[10]));
    assert(rc == P1_SUCCESS);
    rc = P1_Fork("Unblocks7", Unblocks, (void *) "Unblocks7", USLOSS_MIN_STACK,
                 1, 0, &(pid_in[11]));
    assert(rc == P1_SUCCESS);


    int j;
    for (j = 0; j < 12; ++j) {
        USLOSS_Console("HELPER: going to get child %d status\n", j);
        rc = P1GetChildStatus(0, &(pid_out[j]), &status);
        assert(rc == P1_SUCCESS);
        assert(pid_in[j] == pid_out[j]);
        assert(status == expected_status[j]);
    }
    rc = P1_SemFree(sem);
    assert(rc == P1_SUCCESS);
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
    rc = P1_Fork("helper", helper, (void *) 0, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);
    assert(pid == 0);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}