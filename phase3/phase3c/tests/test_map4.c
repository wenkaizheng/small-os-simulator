/*
 * test_map.c
 *  
 *  Tests the P3FrameMap and P3FrameUnmap functions. P3SwapIn uses these functions to fill the page
 *  with its page number.
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
#include"phase2.h"
#include "tester.h"
#include "phase3Int.h"

#define PAGES 30         // # of pages
#define FRAMES 90    // # of frames
#define PAGERS 3        // # of pagers

static char *vmRegion;
static int  pageSize;
//static int child_index;
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
static int Child2(void* p){
    int     j;
    char    *page;
    int     pid;
    //child_index =2;
    Sys_GetPID(&pid);
    Debug("hi (%d) starting.\n", pid);

    // Pages should be filled with their page numbers. 
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child %d reading from page %d @ %p\n",pid, j, page);
        for (int k = 0; k < pageSize; k++) {
            assert (page[k]== j+12);
        }
    }
    Debug("Child2 done.\n");
    return 0;
}
static int Child1(void* p){
    int     j;
    char    *page;
    int     pid;
   // child_index =1;
    Sys_GetPID(&pid);
    Debug("hi (%d) starting.\n", pid);

    // Pages should be filled with their page numbers. 
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child %d reading from page %d @ %p\n",pid, j, page);
        for (int k = 0; k < pageSize; k++) {
            assert (page[k]== j+12);
        }
    }
    Debug("Child1 done.\n");
    return 0;
}
static int
Child(void *arg)
{
    int     j;
    char    *page;
    int     pid;
   // child_index = 0;
    Sys_GetPID(&pid);
    Debug("Child (%d) starting.\n", pid);

    // Pages should be filled with their page numbers. 
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child %d reading from page %d @ %p\n", pid, j, page);
        for (int k = 0; k < pageSize; k++) {
            assert(page[k]== j+12);
        }
    }
    Debug("Child done.\n");
    return 0;
}

int
P4_Startup(void *arg)
{
    int     rc;
    int     pid;
    int     status;

    Debug("P4_Startup starting.\n");
    rc = Sys_VmInit(PAGES, PAGES, FRAMES, PAGERS, (void **) &vmRegion);
    assert(rc ==P1_SUCCESS);


    pageSize = USLOSS_MmuPageSize();
    rc = Sys_Spawn("Child", Child, NULL, USLOSS_MIN_STACK * 4, 3, &pid);
    assert(rc == P1_SUCCESS);
     rc = Sys_Spawn("Child1", Child1, NULL, USLOSS_MIN_STACK * 4, 3, &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Spawn("Child2", Child2, NULL, USLOSS_MIN_STACK * 4, 3, &pid);
    assert(rc == P1_SUCCESS);
    Debug("103 Hi pid %d \n", pid);
    rc = Sys_Wait(&pid, &status);
    Debug("105 pid %d \n", pid);
    assert(rc == P1_SUCCESS);
    assert(status== 0);
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    assert(status == 0);
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    assert(status== 0);
    Debug("Child terminated\n");
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
int P3SwapIn(PID pid, int page, int frame) {
    int rc = 0;
    void *addr;
    Debug("P3SwapIn PID %d page %d frame %d\n", pid, page, frame);
    rc = P3FrameMap(frame, &addr);
    assert(rc== P1_SUCCESS);
    memset(addr, page+12, pageSize);
    rc = P3FrameUnmap(frame);
    assert(rc== P1_SUCCESS);
    rc = P2_Sleep(5);
    Debug("weak up  in here\n");
    assert(rc == P1_SUCCESS);
    return P1_SUCCESS;
}



