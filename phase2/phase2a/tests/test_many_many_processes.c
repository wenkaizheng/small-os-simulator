/*
 * test_basic.c
 *
 * Tests basic functionality of all system calls.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"

/*
 * CheckName
 *
 * Verifies that the process with the specified pid has the specified name.
 * Tests Sys_GetProcInfo.
 */
static int passed = TRUE;

int calculate_factor(int x);



int Child(void *arg) {


    int result = 1;
    int i = (int) arg, j;

    for (j = 1; j < i; ++j) {
        result *= j;
    }



    //  Sys_GetTimeOfDay(&child1Time);
    return result;
}

/*
 * P2_Startup
 *
 * Entry point for this test. Creates the user-level process P3_Startup.
 * Tests P2_Spawn and P2_Wait.
 */

int P2_Startup(void *arg) {
    P2ProcInit();
    int p2Pid = P1_GetPid();
    //USLOSS_Console("%d\n",p2Pid);
    assert(p2Pid == 1);
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);
    int p3Pid;
    int rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3,
                      &p3Pid);
    assert(p3Pid == 2);
    assert(rc == P1_SUCCESS);
    int waitPid, status;
    rc = P2_Wait(&waitPid, &status);
    assert(rc == P1_SUCCESS);
    assert(waitPid == 2);
    assert(status == 12);
    USLOSS_Console("P2 is done\n");
    return strlen(p) + strlen(c);
}

int P3_Startup(void *arg) {
    int rc, waitPid, status;
    int N = 500;
    int input[N];
    int tmp = 5, i;
    for (i = 0; i < N; ++i) {
        input[i] = tmp;
        tmp += 2;
    }

    USLOSS_Console("calculating factors\n");
    int output[N];
    for (i = 0; i < N; ++i) {
        output[i] = calculate_factor(input[i]);
    }

    USLOSS_Console("calculating finish\n");
    int j, childPid, helper[60];
    char buffer[50];
    for (i = 0; i < N-40;) {
        for (j = 0; j < 40; ++j, ++i) {
            sprintf(buffer, "Child No%d", i);
            rc = Sys_Spawn(buffer, Child, (void *)input[i], USLOSS_MIN_STACK,
                   1, &childPid);
            TEST(rc, P1_SUCCESS);
            helper[childPid] = i;
        }
        for (j = 0; j < 40; ++j) {
            rc = Sys_Wait(&waitPid, &status);
            assert(rc == P1_SUCCESS);
            assert(output[helper[waitPid]] == status);
        }
    }


    USLOSS_Console("P3 is done\n");
    Sys_Terminate(12);
    // does not get here

    assert(0);
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);

    return strlen(p) + strlen(c);
}

int calculate_factor(int x) {
    int result = 1;
    int i = x, j;
    for (j = 1; j < i; ++j) {
        result *= j;
    }
    return result;
}


void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}