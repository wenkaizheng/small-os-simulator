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

#define PAGES 3         // # of pages
#define FRAMES 15      // # of frames
#define PAGERS 3        // # of pagers
#define ITERATIONS 10
static char *vmRegion;
static int  pageSize;
static int child_index;
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
static int Child4(void* p){
    int     j;
    char    *page;
    int     pid;
   // int rc;
    Sys_GetPID(&pid);
    Debug("Child4 (%d) starting.\n", pid);
     child_index =4;
    // Pages should be filled with their page numbers.
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child4 %d reading from page %d @ %p\n", pid, j, page);
        for (int k = 0; k < pageSize; k++) {
            child_index =4;
            assert(page[k]== '\0');
        }
    }
    int i;
    Debug("Child4 going to writing \n");
    for (i = 0; i < ITERATIONS; i++) {
        Debug("Child4 %d interation \n", i);
        for (j = 0; j < PAGES; j++) {
          //  rc = Sys_Sleep(1);
            //assert(rc == P1_SUCCESS);
            child_index =4;
            page = vmRegion + j * pageSize;
          //  Debug("Child4 \"%s\" writing to page %d @ %p\n", pid, j, page);
            for (int k = 0; k < pageSize; k++) {
                child_index =4;
                page[k] = 'a';
            }
        }
        for (j = 0; j < PAGES; j++) {
           // rc = Sys_Sleep(1);
            //assert(rc == P1_SUCCESS);
            page = vmRegion + j * pageSize;
           // Debug("Child4 \"%s\" reading from page %d @ %p\n", pid, j, page);
            for (int k = 0; k < pageSize; k++) {
                child_index =4;
                assert(page[k]== 'a');
            }
        }
    }
    Debug("Child4  done.\n");

    return 0;
}
static int Child3(void* p){
    int     j;
    char    *page;
    int     pid;
    Sys_GetPID(&pid);
    Debug("Child3 hi (%d) starting.\n", pid);
   //  child_index =3;
    // Pages should be filled with their page numbers.
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child3 %d reading from page %d @ %p\n",pid, j, page);
        for (int k = 0; k < pageSize; k++) {
            child_index = 2;
            assert (page[k]== j);
        }
    }
    Debug("Child3 done.\n");
    return 0;
}
static int Child2(void* p){
    int     j;
    char    *page;
    int     pid;
    Sys_GetPID(&pid);
    Debug("child2 (%d) starting.\n", pid);
   // child_index =2;
    // Pages should be filled with their page numbers.
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("child2 %d reading from page %d @ %p\n",pid, j, page);
        for (int k = 0; k < pageSize; k++) {
           child_index =2;
            assert (page[k]== j);
        }
    }
    Debug("Child2 done.\n");
    return 0;
}
static int Child(void* p){
    child_index = 0;
    int     pid;
    char    dummy;

    Sys_GetPID(&pid);
    Debug("Child (%d) starting.\n", pid);

    // Read the first page. This should cause an out-of-swap error and we should die.
    dummy = *vmRegion;
    // Should not get here.
    passed = FALSE;
    Debug("Child still alive!!\n");
    return 1;
}
static int
Child1(void *arg)
{

    int     j;
    char    *page;
    int     pid;
    Sys_GetPID(&pid);
    Debug("Child1 (%d) starting.\n", pid);
   // child_index = 1;
    // Pages should be filled with their page numbers.
    for (j = 0; j < PAGES; j++) {
        page = vmRegion + j * pageSize;
        Debug("Child1 %d reading from page %d @ %p\n", pid, j, page);
        for (int k = 0; k < pageSize; k++) {
            child_index =2;
            //USLOSS_Console("Child1 161th %c %c\n",page[k],j%10);
            assert(page[k]== j);

        }
    }
    Debug("Child1 done.\n");
    return 0;
}

int
P4_Startup(void *arg)
{
    int     rc;
    int     pid1, pid;
    int     status;

    Debug("P4_Startup starting.\n");
    rc = Sys_VmInit(PAGES, PAGES, FRAMES, PAGERS, (void **) &vmRegion);
    assert(rc ==P1_SUCCESS);


    pageSize = USLOSS_MmuPageSize();
    rc = Sys_Spawn("Child", Child, NULL, USLOSS_MIN_STACK * 4, 1, &pid1);
    assert(rc == P1_SUCCESS);
     rc = Sys_Spawn("Child1", Child1, NULL, USLOSS_MIN_STACK * 4, 1, &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Spawn("Child2", Child2, NULL, USLOSS_MIN_STACK * 4, 1, &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Spawn("Child3", Child3, NULL, USLOSS_MIN_STACK * 4, 1, &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Spawn("Child4", Child4, NULL, USLOSS_MIN_STACK * 4, 1, &pid);
    assert(rc == P1_SUCCESS);

    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    if(pid == pid1){
        assert(status== -37);
    }else{
        assert(status== 0);
    }
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    if(pid == pid1){
        assert(status== -37);
    }else{
        assert(status== 0);
    }
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("204th %d\n",status);
    if(pid == pid1){
        assert(status== -37);
    }else{
        assert(status== 0);
    }
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    if(pid == pid1){
        assert(status== -37);
    }else{
        assert(status== 0);
    }
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    if(pid == pid1){
        assert(status== -37);
    }else{
        assert(status== 0);
    }
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
    if(child_index == 4){
        child_index = 2;
        Debug("P3SwapIn going to return for child 4");
       return  P3_EMPTY_PAGE;
    }
    if(child_index == 0){
        return P3_OUT_OF_SWAP;
    }


    int rc = 0;
    void *addr;
    Debug("P3SwapIn PID %d page %d frame %d\n", pid, page, frame);
    rc = P3FrameMap(frame, &addr);
    assert(rc== P1_SUCCESS);
    memset(addr, page, pageSize);
    rc = P3FrameUnmap(frame);
    assert(rc== P1_SUCCESS);
    //rc = P2_Sleep(5);
    Debug("wake up  in here\n");
    //assert(rc == P1_SUCCESS);
    return P1_SUCCESS;
}



