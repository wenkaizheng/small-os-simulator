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
    //USLOSS_Console("31th name is %s and %s\n", info.name, name);

    TEST(strcmp(info.name, name), 0);
}
int Child(void *arg)
{
    int pid;
    Sys_GetPID(&pid);
    char buffer[50];
    sprintf(buffer, "Child No%d", pid);
   // USLOSS_Console("child %d\n", pid);
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
    int waitPid ,status;
    rc =P2_Wait(&waitPid,&status);
    assert(rc == P1_SUCCESS);
    assert(waitPid ==2);
    assert(status==12);
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
    for (; i < 47; i++)
    {
        sprintf(buffer, "Child No%d", i + 3);
        // prio 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5....
        // pid 3 4 5 6 7 9 9 10 11 12
        // status 42 43 44 45 46 47 48 49 50 51
        rc = Sys_Spawn(buffer, Child, (void *)42 + i, USLOSS_MIN_STACK, i % 5 + 1, &childPid);
        TEST(rc, P1_SUCCESS);
        //   USLOSS_Console("84th %d\n",childPid);
        TEST(childPid, 3 + i);
    }
    // 12 3 4 5
    int assertArray[47][2]= {{3,42},{4,43},{8,47},{9,48},{13,52},{14,53},{18,57},{19,58},{23,62},{24,63},{28,67},{29,68},
    {33,72},{34,73},{38,77},{39,78},{43,82},{44,83},{48,87},{49,88},{5,44},{10,49},{15,54},{20,59},{25,64},{30,69},
    {35,74},{40,79},{45,84},{6,45},{11,50},{16,55},{21,60},{26,65},{31,70},{36,75},{41,80},{46,85},{7,46},{12,51},{17,56},
    {22,61},{27,66},{32,71},{37,76},{42,81},{47,86}};
    int j = 0;
    int start =0;
    int finish;
    for (; j < 47; j++)
    {
        
        rc = Sys_Wait(&waitPid, &status);
        USLOSS_Console("Pid%d is done and status is %d\n", waitPid, status);
        assert(waitPid == assertArray[j][0]);
        assert(status == assertArray[j][1]);
        Sys_GetTimeOfDay(&finish);
        assert(finish>start);
        USLOSS_Console("The start is %d and finish is %d\n",start,finish);
        start =  finish;
       
        
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