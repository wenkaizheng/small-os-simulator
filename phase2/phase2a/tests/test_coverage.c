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
static int child1Time;
static int child2Time;

static void CheckName(char *name, int pid)
{
    P1_ProcInfo info;

    int rc = Sys_GetProcInfo(pid, &info);
    TEST(rc, P1_SUCCESS);
    TEST(strcmp(info.name, name), 0);
}
int Child(void *arg)
{
    int pid;
    Sys_GetPID(&pid);
    TEST(pid, 3);
    CheckName("Child", 3);
    USLOSS_Console("child\n");
    Sys_GetTimeOfDay(&child1Time);
    return (int)arg;
}
int Child1(void *arg)
{
    int pid;
    Sys_GetPID(&pid);
    TEST(pid, 4);
    CheckName("Child1", 4);
    USLOSS_Console("child1\n");
    Sys_GetTimeOfDay(&child2Time);
    USLOSS_Console("%d %d\n",child1Time,child2Time);
    assert(child1Time < child2Time);
    return (int)arg;
}
/*
 * P2_Startup
 *
 * Entry point for this test. Creates the user-level process P3_Startup.
 * Tests P2_Spawn and P2_Wait.
 */

int P2_Startup(void *arg)
{
    P2ProcInit();
    int p2Pid = P1_GetPid();
    //USLOSS_Console("%d\n",p2Pid);
    assert(p2Pid == 1);
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);
    int p3Pid;
    int rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &p3Pid);
    assert(p3Pid ==2);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P2 is done\n");
    return strlen(p) + strlen(c);
}
int P3_Startup(void *arg)
{
    int rc, waitPid, status, waitPid1, status1;
    int start, finish;

    Sys_GetTimeOfDay(&start);

    int childPid3, childPid4;
    rc = Sys_Spawn("Child", Child, (void *)42, USLOSS_MIN_STACK, 4, &childPid3);
    rc = Sys_Spawn("Child1", Child1, (void *)24, USLOSS_MIN_STACK, 4, &childPid4);
    TEST(rc, P1_SUCCESS);
    assert(childPid3 ==3);
    assert(childPid4 ==4);
    rc = Sys_Wait(&waitPid, &status);
    TEST(status, 42);
    TEST(3,waitPid);
    rc = Sys_Wait(&waitPid1, &status1);
    TEST(4,waitPid1);
    TEST(status1, 24);
    TEST(rc, P1_SUCCESS);
    Sys_GetTimeOfDay(&finish);
    TEST(finish > start, 1);
    USLOSS_Console("start is %d\n",start);
    USLOSS_Console("finish time is %d\n",finish);
    USLOSS_Console("P3 is done and it is last one except sentinel\n");
    Sys_Terminate(11);
    // does not get here

    assert(0);
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);

    return strlen(p) + strlen(c);
}

void test_setup(int argc, char **argv)
{
    // Do nothing.
}

void test_cleanup(int argc, char **argv)
{
    if (passed)
    {
        USLOSS_Console("TEST PASSED.\n");
    }
}
