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

void my_assert(int arg1, int arg2) {
    assert(arg1 == arg2);
}

/*
 * Sleeper
 *
 * Sleeps for the number of seconds in arg and checks that it slept long enough.
 *
 */

int Sleeper(void *arg) {
    int start, end, rc;
    int seconds = (int) arg;
    Sys_GetTimeOfDay(&start);
    rc = Sys_Sleep(seconds);
    assert(rc == 0);
    Sys_GetTimeOfDay(&end);
    my_assert(Slept(start, end) >= seconds, 1);
    my_assert(Slept(start, end) <= seconds+1, 1);
    // USLOSS_Console("The start is %d and end is %d, and second is %d",start,end,seconds);
    return seconds;
}

/*
 * P3_Startup
 *
 * Creates NUM_SLEEPERS children who sleep for random amounts of time.
 *
 */
int
P3_Startup(void *arg) {
    int status, rc;
    int pid = -1;
    int i;
    for (i = 0; i < 46; i++) {
        int duration = i * 5;
        USLOSS_Console("%lld\n", duration);
        rc = Sys_Spawn(MakeName("Sleeper", i), Sleeper, (void *) duration,
                       USLOSS_MIN_STACK, 5, &pid);
        my_assert(rc, P1_SUCCESS);
    }

    for (i = 0; i < 46; i++) {
        rc = Sys_Wait(&pid, &status);
        USLOSS_Console("%d\n", status);
        my_assert(status, i * 5);
        my_assert(rc, P1_SUCCESS);
    }
    return 11;
}

int P2_Startup(void *arg) {
    int rc, waitPid, status, p3Pid;

    P2ClockInit();
    rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3,
                  &p3Pid);
    my_assert(rc, P1_SUCCESS);
    rc = P2_Wait(&waitPid, &status);
    my_assert(rc, P1_SUCCESS);
    my_assert(waitPid, p3Pid);
    my_assert(status, 11);
    P2ClockShutdown();
    PASSED();
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);


    return strlen(p) + strlen(c);
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}
