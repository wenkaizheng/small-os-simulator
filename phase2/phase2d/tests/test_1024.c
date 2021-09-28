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

int child2(void *arg);

int P3_Startup(void *arg) {
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);
    int p3Pid;
    int rc = Sys_Spawn("child2", child2, NULL, 4 * USLOSS_MIN_STACK, 3,
                      &p3Pid);
    assert(p3Pid == 2);
    assert(rc == P1_SUCCESS);
    int waitPid;
    int status;
    rc = Sys_Wait(&waitPid, &status);
    assert(rc == P1_SUCCESS);
    assert(waitPid == 2);
    assert(status == 1024);
    USLOSS_Console("P2 is done\n");
    return strlen(p) + strlen(c);
}



int child2(void *arg) {
    int rc = USLOSS_PsrSet(USLOSS_PsrGet() & ~0x1);
    USLOSS_Console("rc %d", rc);
    assert(0);
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
