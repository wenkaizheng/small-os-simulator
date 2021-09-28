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

static int passed = TRUE;

static int childPid, p3Pid;
#define mode_check() \
    if( (USLOSS_PsrGet()& USLOSS_PSR_CURRENT_MODE)!=1){\
         USLOSS_Console("You are in the user mode");\
         USLOSS_Halt(1);\
    }


int child1(void *arg);
/*
 * CheckName
 *
 * Verifies that the process with the specified pid has the specified name.
 * Tests Sys_GetProcInfo.
 */

static void CheckName(char *name, int pid) 
{
    P1_ProcInfo info;

    int rc = Sys_GetProcInfo(pid, &info);
    assert(rc== P1_SUCCESS);
    assert(strcmp(info.name, name)==0);
}

/*
 * P2_Startup
 *
 * Entry point for this test. Creates the user-level process P3_Startup.
 * Tests P2_Spawn and P2_Wait.
 */

int P3_Startup(void *arg)
{
    int rc, waitPid, status;

    rc = Sys_Spawn("child2", child1, NULL, 4*USLOSS_MIN_STACK, 3, &p3Pid);
    assert(rc== P1_SUCCESS);
    rc = Sys_Wait(&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(status==11);
    assert(waitPid== p3Pid);
    PASSED();
    return 0;
}

/*
 * Child
 *
 * Checks its pid and those of its ancestors.
 * Tests Sys_GetPID.
 */

int Child(void *arg) {
    int pid;
    Sys_GetPID(&pid);
    assert(pid== childPid);
    CheckName("child2", p3Pid);
    return (int) arg;
}

/*
 * P3_Startup
 *
 * Initial user-level process. 
 * Tests Sys_GetTimeOfDay, Sys_Spawn, Sys_Wait, and Sys_Terminate.
 */

int child1(void *arg) {
    int rc, waitPid, status;
    int start, finish;

    Sys_GetTimeOfDay(&start);

    rc = Sys_Spawn("Child", Child, (void *) 42, USLOSS_MIN_STACK, 3, &childPid);
    assert(rc==P1_SUCCESS);
    rc = Sys_Wait(&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(status== 42);
    assert(waitPid== childPid);
    Sys_GetTimeOfDay(&finish);
    assert(finish > start);
    Sys_Terminate(11);
    // does not get here

    assert(0);
    DumpProcesses();
    char * p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);


    return strlen(p)+strlen(c);
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}
