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

int P2_Startup(void *arg) {
    P2ProcInit();
    int p2Pid = P1_GetPid();
    //USLOSS_Console("%d\n",p2Pid);
    assert(p2Pid == 1);
    DumpProcesses();
    char *p = MakeName("%d", 1);
    char *c = ErrorCodeToString(-1);
    int p3Pid;
    int rc = P1_Fork("P3_Startup", P3_Startup, NULL, 4 * USLOSS_MIN_STACK, 3,
                     0, &p3Pid);
    assert(p3Pid == 2);
    assert(rc == P1_SUCCESS);
    int waitPid;
    int status;
    rc = P1_Join(0, &waitPid, &status);
    assert(rc == P1_SUCCESS);
    assert(waitPid == 2);
    assert(status == 1024);
    USLOSS_Console("P2 is done\n");
    return strlen(p) + strlen(c);
}

int P4_Startup(void *arg) {
    USLOSS_Console("43th\n");
    int rc = P1_GetPid();
    USLOSS_Console("45th%d\n", rc);
    return 0;
}

int P3_Startup(void *arg) {
    int p4Pid;
    int rc = USLOSS_PsrSet(USLOSS_PsrGet() & ~0x1);
    assert(rc == USLOSS_DEV_OK);
    rc = P2_Spawn("P4_Startup", P4_Startup, NULL, 4 * USLOSS_MIN_STACK, 3,
                  &p4Pid);
    assert(rc != P1_SUCCESS);
    assert(1);
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
