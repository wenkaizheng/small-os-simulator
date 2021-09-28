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

static void CheckName(char *name, int pid)
{
    P1_ProcInfo info;

    int rc = Sys_GetProcInfo(pid, &info);
    TEST(rc, P1_SUCCESS);
    USLOSS_Console("31th name is %s and %s\n", info.name, name);

    TEST(strcmp(info.name, name), 0);
}
int Child(void *arg)
{
    int pid;
    Sys_GetPID(&pid);
    char buffer[50];
    sprintf(buffer, "Child No%d", pid);
    USLOSS_Console("child %d\n", pid);
    CheckName(buffer, pid);

    //  Sys_GetTimeOfDay(&child1Time);
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
    assert(p3Pid == 2);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("P2 is done\n");
    return strlen(p) + strlen(c);
}
int P3_Startup(void *arg)
{
    int rc, waitPid, status;
    //int start, finish;

    // Sys_GetTimeOfDay(&start);

    int childPid;
    int i = 0;
    char buffer[50];
    for (; i < 10; i++)
    {
        sprintf(buffer, "Child No%d", i + 3);
        // 1 2 3 1 2 3 1 2 3 1
        // 3 4 5 6 7 9 9 10 11 12
        // 42 43 44 45 46 47 48 49 50 51
        rc = Sys_Spawn(buffer, Child, (void *)42 + i, USLOSS_MIN_STACK, i % 3 + 1, &childPid);
        TEST(rc, P1_SUCCESS);
        //   USLOSS_Console("84th %d\n",childPid);
        TEST(childPid, 3 + i);
    }
    int j = 0;
    for (; j < 10; j++)
    {
        rc = Sys_Wait(&waitPid, &status);
        USLOSS_Console("Pid%d is done and status is %d\n", waitPid, status);
        if(j==0) {
            TEST(status,42);
        }
        else if(j==1) {
            TEST(status,43);
        }
        else if(j==2){
            TEST(status,45);
        }
        else if(j==3){
            TEST(status,46);
        }
        else if(j==4){
            TEST(status,48);
        }
        else if(j==5){
            TEST(status,49);
        }
        else if(j==6){
            TEST(status,51);
        }
        else if (j==7){
            TEST(status,44);
        }
        else if(j==8) {
            TEST(status,47);
        }
        else {
             TEST(status,50);
        }
    }

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
