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
static void
GetTimeOfDayStub(USLOSS_Sysargs *sysargs) {
    int rc = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, (int *) &(sysargs->arg1));
    assert(rc == USLOSS_DEV_OK);
}
int P2_Startup(void *arg)
{
    assert(P2_SetSyscallHandler(0,GetTimeOfDayStub)==P2_INVALID_SYSCALL);
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);
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
