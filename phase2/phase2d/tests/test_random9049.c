/*
 * test_sleep.c
 */
#include <assert.h>
#include <usloss.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"

#define NUM_SLEEPERS 10

static int passed = TRUE;
int Slept(int start, int end) {
    return (end - start) / 1000000;
}

/*
 * Sleeper
 *
 * Sleeps for the number of seconds in arg and checks that it slept long enough.
 *
 */
void my_assert(int arg1, int arg2) {
    assert(arg1 == arg2);
}

int Sleeper(void *arg) {
    int start, end, rc;
    int seconds = (int) arg;
    Sys_GetTimeOfDay(&start);
    rc = Sys_Sleep(seconds);
    my_assert(rc, 0);
    Sys_GetTimeOfDay(&end);
    my_assert(Slept(start, end) >= seconds, 1);
    my_assert(Slept(start, end) <= seconds+1, 1);
    USLOSS_Console("36:%d start:%d end:%d\n", seconds, start, end);
    return 0;
}

int Helper1(void *arg) {
    int status, rc;
    int pid = -1;
    srandom(9049);
    int i;
    for (i = 0; i < NUM_SLEEPERS; i++) {
        int duration = (random() % 11) * 13 + (random() % 3) + 1;
        rc = Sys_Spawn(MakeName("helper1_sleeper", i), Sleeper,
                       (void *) duration, USLOSS_MIN_STACK, (i % 5) + 1, &pid);
        my_assert(rc, P1_SUCCESS);
    }

    for (i = 0; i < NUM_SLEEPERS; i++) {
        rc = Sys_Wait(&pid, &status);
        my_assert(rc, P1_SUCCESS);
    }
    return 12;
}

int Helper2(void *arg) {
    int status, rc;
    int pid = -1;
    int i;
    for (i = 0; i < NUM_SLEEPERS; i++) {
        int duration = (random() % 7) * 17 + (random() % 5) + 3;
        rc = Sys_Spawn(MakeName("helper2_sleeper", i), Sleeper,
                       (void *) duration, USLOSS_MIN_STACK, (i % 5) + 1, &pid);
        USLOSS_Console("64th %d\n", rc);
        my_assert(rc, P1_SUCCESS);
    }

    for (i = 0; i < NUM_SLEEPERS; i++) {
        rc = Sys_Wait(&pid, &status);
        my_assert(rc, P1_SUCCESS);
    }
    return 13;
}

/*
 * P3_Startup
 *
 * Creates NUM_SLEEPERS children who sleep for random amounts of time.
 *
 */
int
P3_Startup(void *arg) {
    int rc1;
    int rc2;
    int pid;
    rc1 = Sys_Spawn("Helper1", Helper1, NULL, USLOSS_MIN_STACK, 2, &pid);
    my_assert(rc1, P1_SUCCESS);
    rc2 = Sys_Spawn("Helper2", Helper2, NULL, USLOSS_MIN_STACK, 2, &pid);
    my_assert(rc2, P1_SUCCESS);
    int waitPid1;
    int waitPid2;
    int status1;
    int status2;
    rc1 = Sys_Wait(&waitPid1, &status1);
    my_assert(rc1, P1_SUCCESS);
    rc2 = Sys_Wait(&waitPid2, &status2);
    my_assert(rc2, P1_SUCCESS);
    return 0;

}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}
