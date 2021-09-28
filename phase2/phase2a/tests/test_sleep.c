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
int P4_Startup(void *arg)
{
   
    
    return -1;
}
int P3_Startup(void *arg)
{
    int p4Pid;
    int rc = Sys_Spawn("P4_Startup", P4_Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &p4Pid);
    assert(rc == P1_SUCCESS);
    assert(p4Pid ==3);
    int waitPid;
    int status;
    rc = Sys_Wait(&waitPid,&status);
    assert(waitPid == 3);
    assert(status == -1);
    assert(rc == P1_SUCCESS);
    rc =  Sys_Sleep(1000);
    assert(rc ==P2_INVALID_SYSCALL);
    return 0;
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
