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

static int testing[15] = {75, 80, 85, 90, 95, 100, 105, 110, 130, 143, 156, 169,
                          182, 195, 208};

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
int P3_Startup(void *arg) {
    int status, rc;
    int pid = -1;
    int i;
    for (i = 0; i < 23; i++) {
        int duration = i * 5;
        USLOSS_Console("%lld\n", duration);
        rc = Sys_Spawn(MakeName("Sleeper", i), Sleeper, (void *) duration,
                       USLOSS_MIN_STACK, 5, &pid);
        my_assert(rc, P1_SUCCESS);
    }

    for (i = 0; i < 15; i++) {
        rc = Sys_Wait(&pid, &status);
        USLOSS_Console("61th %d\n", status);
        my_assert(status, i * 5);
        my_assert(rc, P1_SUCCESS);
    }
    for (i = 0; i < 7; i++) {
        int duration = (i + 10) * 13;
        // USLOSS_Console("%lld\n",duration);
        rc = Sys_Spawn(MakeName("Sleeper", i), Sleeper, (void *) duration,
                       USLOSS_MIN_STACK, 5, &pid);
        my_assert(rc, P1_SUCCESS);
    }
    for (i = 0; i < 15; i++) {
        rc = Sys_Wait(&pid, &status);
        USLOSS_Console("73th %d %d\n", status, testing[i]);
        my_assert(status, testing[i]);
        my_assert(rc, P1_SUCCESS);
    }
    return 11;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}
