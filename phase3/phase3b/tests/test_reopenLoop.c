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

#define PAGES 4
#define PAGE_1 1000
typedef int PID;
typedef int SID;

static char *vmRegion;
static int pageSize;
static int pageSize_1;

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

    if (debugging)
    {
        va_start(ap, fmt);
        USLOSS_VConsole(fmt, ap);
    }
}
static int
Writer1(void *arg)
{
    SID sid = (SID)arg;
    int rc;
    PID pid;

    Sys_GetPID(&pid);
    Debug("Writer (%d) starting.\n", pid);
    for (int i = 0; i < PAGE_1; i+=10)
    {
        char *page = vmRegion + i * pageSize - 1;
        Debug("Writing page %d.\n", i);
        for (int j = 0; j < pageSize_1; j++)
        {
            page[j] = i + 1;
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
    SID sid = (SID)arg;
    int rc;
    PID pid;

    Sys_GetPID(&pid);
    rc = Sys_SemP(sid);
    assert(rc == P1_SUCCESS);
    Debug("Reader (%d) starting.\n", pid);
    for (int i = 0; i < PAGE_1; i+=10)
    {
        char *page = vmRegion + i * pageSize_1;
        Debug("Reading page %d.\n", i);
        for (int j = 0; j < pageSize_1; j++)
        {
            assert(page[j] == i + 1);
        }
    }
    Debug("Reader1 done.\n", pid);
    return 0;
}

static int
Writer(void *arg)
{
    SID sid = (SID)arg;
    int rc;
    PID pid;

    Sys_GetPID(&pid);
    Debug("Writer (%d) starting.\n", pid);
    for (int i = 0; i < PAGES; i++)
    {
        char *page = vmRegion + i * pageSize;
        Debug("Writing page %d.\n", i);
        for (int j = 0; j < pageSize; j++)
        {
            page[j] = i + 1;
        }
    }

    rc = Sys_SemV(sid);
    assert(rc == P1_SUCCESS);
    Debug("Writer done.\n", pid);
    return 0;
}
static int
Reader(void *arg)
{
    SID sid = (SID)arg;
    int rc;
    PID pid;
   // int i;
    PID child;
    Sys_GetPID(&pid);
    rc = Sys_SemP(sid);
    assert(rc == P1_SUCCESS);
    Debug("Reader (%d) starting.\n", pid);
    for (int i = 0; i < PAGES; i++)
    {
        char *page = vmRegion + i * pageSize;
        Debug("Reading page %d.\n", i);
        for (int j = 0; j < pageSize; j++)
        {
            assert(page[j] == i + 1);
        }
    }
    Debug("Reader done.\n", pid);
    TEST(P3_vmStats.faults, 0);
    Sys_VmShutdown();
    // we are going to do re init in here
    rc = Sys_VmInit(PAGES, PAGES, PAGES, 0, (void **)&vmRegion);
    TEST(rc, P1_SUCCESS);

    pageSize = USLOSS_MmuPageSize();
    int j;
    for (j = 0; j < 15; j++)
    {
        char buffer[P1_MAXNAME+1];
        sprintf(buffer, "Reader %d",j );
        rc = Sys_Spawn(buffer, Reader1, (void *)sid, USLOSS_MIN_STACK * 2, 3, &pid);
        assert(rc == P1_SUCCESS);
        sprintf(buffer, "Writer %d",j );
        rc = Sys_Spawn(buffer, Writer1, (void *)sid, USLOSS_MIN_STACK * 2, 3, &pid);
        assert(rc == P1_SUCCESS);
    }
    for (j = 0; j< 30; j++)
    {
        rc = Sys_Wait(&pid, &child);
        USLOSS_Console("161th\n");
        assert(rc == P1_SUCCESS);
    }
    TEST(P3_vmStats.faults, 0);
    Sys_VmShutdown();
    PASSED();

    return 0;
}

int P4_Startup(void *arg)
{
    int i;
    int rc;
    PID pid;
    PID child;
    SID sid;

    Debug("P4_Startup starting.\n");
    rc = Sys_VmInit(PAGES, PAGES, PAGES, 0, (void **)&vmRegion);
    TEST(rc, P1_SUCCESS);

    rc = Sys_SemCreate("ready", 0, &sid);
    assert(rc == P1_SUCCESS);

    pageSize = USLOSS_MmuPageSize();

    rc = Sys_Spawn("Reader", Reader, (void *)sid, USLOSS_MIN_STACK * 2, 2, &pid);
    assert(rc == P1_SUCCESS);

    rc = Sys_Spawn("Writer", Writer, (void *)sid, USLOSS_MIN_STACK * 2, 2, &pid);
    assert(rc == P1_SUCCESS);

    for (i = 0; i < 2; i++)
    {
        rc = Sys_Wait(&pid, &child);
        USLOSS_Console("199th\n");
        assert(rc == P1_SUCCESS);
    }

    PASSED();
    Debug("P4_Startup done.\n");
    return 0;
}

void test_setup(int argc, char **argv)
{
}

void test_cleanup(int argc, char **argv)
{
    if (passed)
    {
        USLOSS_Console("TEST PASSED.\n");
    }
}