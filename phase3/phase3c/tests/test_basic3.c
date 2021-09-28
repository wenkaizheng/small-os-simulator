/*
 * basic.c
 *  
 *  Basic test case for Phase 3 Part C. It creates two processes, "A" and "B". 
 *  Each process has two pages and there are four frames so that all pages fit in memory.
 *  Each process writes its name into the first byte of each of its pages, sleeps for one
 *  second (to give the other process time to run), then verifies that the first byte
 *  of each page is correct. It then iterates a fixed number of times.
 *
 *  You can change the number of pages and iterations by changing the macros below. You
 *  can add more processes by adding more names to the "names" array, e.g. "C". The
 *  code will adjust the number of frames accordingly.
 *
 *  It makes liberal use of the "assert" function because it will dump core when it fails
 *  allowing you to easily look at the state of the program and figure out what went wrong,
 *  and because I'm lazy.
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
#include "phase3Int.h"

#define PAGES 2         // # of pages per process (be sure to try different values)
#define ITERATIONS 30
#define PAGERS 2        // # of pagers

static char *vmRegion;
static char *names[] ={"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};   // names of children, add more names to create more children
static int  pageSize;

static int passed = FALSE;
#define DEBUG
#ifdef DEBUG
int debugging = 1;
#else
int debugging = 0;
#endif /* DEBUG */

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
Child(void *arg)
{
    volatile char *name = (char *) arg;
    int     i,j;
    char    *page;
    int     rc;
    int     pid;

    Sys_GetPID(&pid);
    Debug("Child \"%s\" (%d) starting.\n", name, pid);

    // The first time a page is read it should be full of zeros.
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("74 Child \"%s\" reading zeros from page %d @ %p\n", name, j, page);
        for (int k = 0; k < pageSize; k++) {
            assert(page[k] =='\0');
        }
    }    
    int n = random();
    for (i = 0; i < ITERATIONS; i++) {
        for (j = PAGES-1; j > 0; j--) {
            rc = Sys_Sleep(2);
            assert(rc == P1_SUCCESS);
            page = vmRegion + j * pageSize;
            Debug("84 Child \"%s\" writing to page %d @ %p\n", name, j, page);
            for (int k = pageSize-1; k >0; k--) {
                page[k] = *name+n%7;
            }
            Debug("88 Child \"%s\" reading from page %d @ %p\n", name, j, page);
            for (int k = pageSize-1; k >0; k--) {
                assert(page[k]== *name+n%7);
            }
        }
       
    }
    Debug("Child \"%s\" done.\n", name);
    return 0;
}


int
P4_Startup(void *arg)
{
    srandom(2147444799);
    int     i;
    int     rc;
    int     pid;
    int     status;
    
    int     numChildren = sizeof(names) / sizeof(char *);

    Debug("P4_Startup starting.\n");
    rc = Sys_VmInit(PAGES, PAGES, numChildren * PAGES, PAGERS, (void **) &vmRegion);
    assert(rc== P1_SUCCESS);


    pageSize = USLOSS_MmuPageSize();
    for (i = 0; i < numChildren; i++) {
        rc = Sys_Spawn(names[i], Child, (void *) names[i], USLOSS_MIN_STACK * 4, random()%4+1, &pid);
        assert(rc == P1_SUCCESS);
    }
    for (i = 0; i < numChildren; i++) {
        rc = Sys_Wait(&pid, &status);
        assert(rc == P1_SUCCESS);
        assert(status== 0);
    }
    Debug("Children terminated\n");
    Sys_VmShutdown();
    PASSED();
    return 0;
}


void test_setup(int argc, char **argv) {
}

void test_cleanup(int argc, char **argv) {
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}

// Phase 3d stubs

#include "phase3Int.h"

int P3SwapInit(int pages, int frames) {return P1_SUCCESS;}
int P3SwapShutdown(void) {return P1_SUCCESS;}
int P3SwapFreeAll(PID pid) {return P1_SUCCESS;}
int P3SwapOut(int *frame) {return P1_SUCCESS;}
int P3SwapIn(PID pid, int page, int frame) {return P3_EMPTY_PAGE;}