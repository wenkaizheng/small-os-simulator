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

int Child(void *arg)
{

    return (int)arg;
}


int P3_Startup(void *arg)
{
    USLOSS_Console("27th\n");
    int i=0;
    char buffer[50];
    int rc;
    int childPid;
    for(;i<47;i++){
        sprintf(buffer, "Child No%d", i + 3);
        
        rc = Sys_Spawn(buffer, Child, (void *) 3+i, USLOSS_MIN_STACK, 2, &childPid);
        USLOSS_Console("%d %d\n",childPid,3+i);
        assert(childPid == 3+i);
        assert(rc == P1_SUCCESS);
        
    }
    rc = Sys_Spawn("more", Child, (void *) i, USLOSS_MIN_STACK, 2, &childPid);
    assert(rc ==P1_TOO_MANY_PROCESSES);
    int j=0;
    int waitPid;
    int status;
    for(;j<47;j++){
        rc = Sys_Wait(&waitPid, &status);
        USLOSS_Console("Pid%d is done and status is %d\n", waitPid, status);
    }
   
    return 0;
}
int P2_Startup(void *arg)
{
    P2ProcInit();
    int pid;
    int rc = P2_Spawn("iunwnwnvwnvowvnwvnonvonvowvnonoqnvonqoinoicnqovnqiovnqonoqvnoqnoqnvoqnvccqccqccacacacaccacsvvcscscscscscscscssvsvsvsvs"
    , P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &pid);
    assert(rc ==P1_NAME_TOO_LONG);

    rc = P2_Spawn(NULL
    , P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &pid);
    assert(rc == P1_NAME_IS_NULL);

    rc = P2_Spawn("ppp"
    , P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &pid);
    rc = P2_Spawn("ppp"
    , P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &pid);
    assert(rc == P1_DUPLICATE_NAME);
    
    rc =P2_Spawn("pppq"
    , P3_Startup, NULL, USLOSS_MIN_STACK-1, 3, &pid);
    assert(rc == P1_INVALID_STACK);

    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);
    USLOSS_Console("56th\n");
    return strlen(p)+strlen(c);
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
