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

#define PAGES 400

typedef int PID;
typedef int SID;

static char *vmRegion;
static int pageSize;

#define DEBUG
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
MaxPid(void *arg)
{
    
    USLOSS_Console("49 pid done\n");
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
    for (int i = 0; i < PAGES; i+=3)
    {
        char *page = vmRegion + i * pageSize;
        Debug("Writing page %d.\n", i);
        for (int j = 0; j < pageSize; j++)
        {
            page[j] = (i + 1) % 127;
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

    Sys_GetPID(&pid);
    rc = Sys_SemP(sid);
    assert(rc == P1_SUCCESS);
    Debug("Reader (%d) starting.\n", pid);
    for (int i = 0; i < PAGES; i+=3)
    {
        char *page = vmRegion + i * pageSize;
        Debug("Reading page %d.\n", i);
        for (int j = 0; j < pageSize; j++)
        {
           
            assert(page[j] == (i + 1) %127);
        }
    }
    Debug("Reader done.\n", pid);
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
     
    for (int i = 0; i < 21;i++)
    {
        char buffer[P1_MAXNAME+1];
        sprintf(buffer, "Reader %d",i ); 

        rc = Sys_Spawn(buffer, Reader, (void *)sid, USLOSS_MIN_STACK * 2, 2, &pid);
        assert(rc == P1_SUCCESS);
        
        sprintf(buffer, "Writer %d",i ); 

        rc = Sys_Spawn(buffer, Writer, (void *)sid, USLOSS_MIN_STACK * 2, 2, &pid);
        assert(rc == P1_SUCCESS);
    }
    rc = Sys_Spawn("49pid", MaxPid, (void *)sid, USLOSS_MIN_STACK * 2, 1, &pid);
    assert(rc == P1_SUCCESS);
    for (i = 0; i < 42; i++)
    {
        rc = Sys_Wait(&pid, &child);
        assert(rc == P1_SUCCESS);
    }
    assert(P3_vmStats.faults == 0);
    Sys_VmShutdown();
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