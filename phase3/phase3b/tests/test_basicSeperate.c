/*
 * basic.c
 *  
 *  Basic test case for Phase 3a. It creates two processes, Writer and Reader. 
 *  Each process has four pages and there are four frames. Phase 3a implements identity page
 *  tables so that page x -> frame x for all processes. Writer writes x+1 to every byte of 
 *  page x, then Reader reads the pages and verifies their contents.
 *
 */
#include <usyscall.h>
#include <libuser.h>
#include <assert.h>
#include <usloss.h>
#include <stdlib.h>
#include <phase3.h>
#include <stdarg.h>
#include <unistd.h>

#include "tester.h"

#define PAGES 100        

typedef int PID;
typedef int SID;

static char *vmRegion;
static int  pageSize;

#ifdef DEBUG
int debugging = 1;
#else
int debugging = 0;
#endif /* DEBUG */


static int passed = FALSE;

static void
Debug(char *fmt, ...)
{
    va_list ap;

    if (debugging) {
        va_start(ap, fmt);
        USLOSS_VConsole(fmt, ap);
    }
}

static int
Writer1(void *arg)
{
    SID     sid = (SID) arg;
    int     rc;
    PID     pid;

    Sys_GetPID(&pid);
    Debug("Writer (%d) starting.\n", pid);
    for (int i = 50; i < 100; i++) {
        char *page = vmRegion + i * pageSize;
        Debug("Writing page %d.\n", i);
        for (int j = 0; j < pageSize; j+=2) {
            page[j] = i+1;
        }
    }

    rc = Sys_SemV(sid);
    assert(rc == P1_SUCCESS);
    Debug("Writer1 done.\n", pid);
    return 0;
}
static int
Reader1(void *arg)
{
    SID     sid = (SID) arg;
    int     rc;
    PID     pid;

    Sys_GetPID(&pid);
    rc = Sys_SemP(sid);
    assert(rc == P1_SUCCESS);
    Debug("Reader (%d) starting.\n", pid);
    for (int i = 50; i < 100; i++) {
        char *page = vmRegion + i * pageSize;
        Debug("Reading page %d.\n", i);
        for (int j = 0; j < pageSize; j+=2) {
            assert(page[j] == i+1);
        }
    }
    Debug("Reader1 done.\n", pid);
    return 0;
}



static int
Writer0(void *arg)
{
    SID     sid = (SID) arg;
    int     rc;
    PID     pid;

    Sys_GetPID(&pid);
    Debug("Writer (%d) starting.\n", pid);
    for (int i = 0; i < 50; i++) {
        char *page = vmRegion + i * pageSize;
        Debug("Writing page %d.\n", i);
        for (int j = 0; j < pageSize; j++) {
            page[j] = i+1;
        }
    }

    rc = Sys_SemV(sid);
    assert(rc == P1_SUCCESS);
    Debug("Writer0 done.\n", pid);
    return 0;
}
static int
Reader0(void *arg)
{
    SID     sid = (SID) arg;
    int     rc;
    PID     pid;

    Sys_GetPID(&pid);
    rc = Sys_SemP(sid);
    assert(rc == P1_SUCCESS);
    Debug("Reader (%d) starting.\n", pid);
    for (int i = 0; i < 50; i++) {
        char *page = vmRegion + i * pageSize;
        Debug("Reading page %d.\n", i);
        for (int j = 0; j < pageSize; j++) {
            assert(page[j] == i+1);
        }
    }
    Debug("Reader0 done.\n", pid);
    return 0;
}


int
P4_Startup(void *arg)
{
    int     i;
    int     rc;
    PID     pid;
    PID     child;
    SID     sid;

    Debug("P4_Startup starting.\n");
    rc = Sys_VmInit(PAGES, PAGES, PAGES, 0, (void **) &vmRegion);
    TEST(rc, P1_SUCCESS);

    rc = Sys_SemCreate("ready", 0, &sid);
    assert(rc == P1_SUCCESS);

    pageSize = USLOSS_MmuPageSize();

    rc = Sys_Spawn("Reader0", Reader0, (void *) sid, USLOSS_MIN_STACK * 2, 2, &pid);
    assert(rc == P1_SUCCESS);

    rc = Sys_Spawn("Writer0", Writer0, (void *) sid, USLOSS_MIN_STACK * 2, 2, &pid);
    assert(rc == P1_SUCCESS);

    rc = Sys_Spawn("Reader1", Reader1, (void *) sid, USLOSS_MIN_STACK * 2, 2, &pid);
    assert(rc == P1_SUCCESS);

    rc = Sys_Spawn("Writer1", Writer1, (void *) sid, USLOSS_MIN_STACK * 2, 2, &pid);
    assert(rc == P1_SUCCESS);

    for (i = 0; i < 4; i++) {
        rc = Sys_Wait(&pid, &child);
        assert(rc == P1_SUCCESS);
    }
    TEST(P3_vmStats.faults, 0);
    Sys_VmShutdown();
    PASSED();
    Debug("P4_Startup done.\n");
    return 0;
}


void test_setup(int argc, char **argv) {
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }

}