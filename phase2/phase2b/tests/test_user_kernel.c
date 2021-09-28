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
    //USLOSS_Console("%s:%d: %d %d\n", __FUNCTION__, __LINE__, arg1, arg2);
    TEST(arg1, arg2);
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
    TEST(Slept(start, end) >= seconds, 1);
    my_assert(Slept(start, end) >= seconds, 1);
    my_assert(Slept(start, end) <= seconds+1, 1);
    USLOSS_Console("36:%d start:%d end:%d\n", seconds, start, end);
    return 0;
}

int Sleeper_kernel(void *arg) {
    int start, end, rc;
    int seconds = (int) arg;
    // Sys_GetTimeOfDay(&start);
    USLOSS_Console("50th %d\n", seconds);
    rc = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, (int *) &(start));
    assert(rc == USLOSS_DEV_OK);

    rc = P2_Sleep(seconds);
    assert(rc == 0);
    USLOSS_Console("59: %d\n", seconds);

    rc = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, (int *) &(end));
    TEST(Slept(start, end) >= seconds, 1);

    //TEST(Slept(start, end) >= seconds, 1); 
    my_assert(Slept(start, end) >= seconds, 1);
    USLOSS_Console("36:%d start:%d end:%d\n", seconds, start, end);
    return 0;
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
    for (i = 0; i < NUM_SLEEPERS; i++) {
        //  int duration = random() % 10;
        rc = Sys_Spawn(MakeName("user_sleeper", i), Sleeper, (void *) i,
                       USLOSS_MIN_STACK, 5, &pid);
        TEST(rc, P1_SUCCESS);
        my_assert(rc, P1_SUCCESS);
    }

    for (i = 0; i < NUM_SLEEPERS; i++) {
        rc = Sys_Wait(&pid, &status);
        my_assert(rc, P1_SUCCESS);
    }
    USLOSS_Console("GET ALL\n");
    return 11;
}

int
kernel_Startup(void *arg) {
    int status, rc;
    int pid = -1;
    int i;

    for (i = 0; i < NUM_SLEEPERS; i++) {
        // int duration = random() % 10;
        USLOSS_Console("104th\n");
        rc = P1_Fork(MakeName("kernal_sleeper", i), Sleeper_kernel, (void *) i,
                     USLOSS_MIN_STACK, 5, 0, &pid);
        my_assert(rc, P1_SUCCESS);
    }
    for (i = 0; i < NUM_SLEEPERS; i++) {
        USLOSS_Console("111th: %d\n", i);
        DumpProcesses();
        rc = P1_Join(0, &pid, &status);
        USLOSS_Console("112th: %d\n", i);
        TEST(rc, P1_SUCCESS);
        my_assert(rc, P1_SUCCESS);
    }
    return 11;
}

int P2_Startup(void *arg) {
    int rc, waitPid, status, p3Pid, p4Pid;

    P2ClockInit();
    rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3,
                  &p3Pid);
    my_assert(rc, P1_SUCCESS);

    rc = P1_Fork("kernel_Startup", kernel_Startup, NULL, 4 * USLOSS_MIN_STACK,
                 3, 0, &p4Pid);
    my_assert(rc, P1_SUCCESS);


    rc = P2_Wait(&waitPid, &status);
    my_assert(rc, P1_SUCCESS);
    my_assert(waitPid, p3Pid);
    my_assert(waitPid, p3Pid);
    my_assert(status, 11);


    rc = P1_Join(0, &waitPid, &status);
    my_assert(rc, P1_SUCCESS);
    my_assert(waitPid, p4Pid);
    my_assert(status, 11);


    P2ClockShutdown();
    PASSED();
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);


    return strlen(p) + strlen(c);
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
