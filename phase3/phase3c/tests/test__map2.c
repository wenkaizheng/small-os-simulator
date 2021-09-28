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

#define PAGES 10         // # of pages
#define FRAMES PAGES    // # of frames
#define PAGERS 2        // # of pagers

static char *vmRegion;
static int  pageSize;

static int passed = FALSE;
static int pages[10]={2,4,8,16,32,64,128,256,512,1024};
static int index_page;
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
          //  USLOSS_Console("62 th %d %d\n",page[k],pages[index_page]);
            assert(page[k] == (pages[index_page] & 0x7f));
        }
        index_page++;
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
    assert(rc== P1_SUCCESS);


    pageSize = USLOSS_MmuPageSize();
    rc = Sys_Spawn("Child", Child, NULL, USLOSS_MIN_STACK * 4, 3, &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    assert(status == 0);
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
int hi(void* p){
   assert(p==NULL);
   USLOSS_Console("hi\n");
   return 0;
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
    Debug("before %p\n",(char*)addr);
    rc = P3FrameMap(frame, &addr);
    assert(rc== P1_SUCCESS);
    Debug("after %p\n",(char*)addr);
    memset(addr, (pages[index_page] & 0x7f), pageSize);
    rc = P3FrameUnmap(frame);
    assert(rc==P1_SUCCESS);
    int Pid;
    rc = P1_Fork("test_process", hi, NULL, 4 * USLOSS_MIN_STACK,
                     1, 0, &Pid);
    rc = P2_Sleep(5);
    Debug("weak up  in here\n");
    assert(rc == P1_SUCCESS);
    return P1_SUCCESS;
}



