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

#include "tester.h"
#include "phase3Int.h"

#define PAGES 2         // # of pages
#define FRAMES PAGES    // # of frames
#define PAGERS 2        // # of pagers

static char *vmRegion;
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
    int     j;
    char    *page;
    int     pid;

    Sys_GetPID(&pid);
    Debug("Child (%d) starting.\n", pid);

    // Pages should be filled with their page numbers.
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child reading from page %d @ %p\n", j, page);
        for (int k = 0; k < pageSize; k++) {
            TEST(page[k], j);
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
   // rc = Sys_VmInit(PAGES, PAGES, FRAMES, 0, (void **) &vmRegion);
    //assert(rc == P3_INVALID_NUM_PAGERS);
    rc = Sys_VmInit(PAGES, PAGES, FRAMES, PAGERS, (void **) &vmRegion);
    USLOSS_Console("78th %d\n",rc);
    assert(rc== P1_SUCCESS);


    pageSize = USLOSS_MmuPageSize();
    rc = Sys_Spawn("Child", Child, NULL, USLOSS_MIN_STACK * 4, 3, &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    TEST(status, 0);
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
    Debug("P3SwapIn PID %d page %d frame %d.\n", pid, page, frame);
    rc = P3FrameMap(frame, &addr);
    assert(rc== P1_SUCCESS);
    memset(addr, page, pageSize);
    rc = P3FrameUnmap(frame);
    assert(rc== P1_SUCCESS);

    void* i ;
    rc = P3FrameMap(-1, &i);
    assert(rc ==P3_INVALID_FRAME);

    rc = P3FrameMap(4, &i);
    assert(rc ==P3_INVALID_FRAME);

    rc = P3FrameUnmap(4);
    assert(rc ==P3_INVALID_FRAME);

    rc = P3FrameUnmap(-1);
    assert(rc ==P3_INVALID_FRAME);

    rc = P3FrameMap(0, &i);
    assert(rc ==P1_SUCCESS);

    rc = P3FrameMap(1, &i);
    assert(rc ==P1_SUCCESS);

    rc = P3FrameMap(1, &i);
    USLOSS_Console("%d\n", rc);
    assert(rc ==P3_OUT_OF_PAGES);

    return P1_SUCCESS;
}