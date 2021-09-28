/*
 * Tests freeing a semaphore that has a process blocked on it
 */

#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <assert.h>
#include <libuser.h>
#include <libdisk.h>

#include "tester.h"
#include "phase2Int.h"

static int sem;

static int 
Free(void *arg){
    USLOSS_Console("Free: Running.\n");
    USLOSS_Console("Free: Calling SemFree(%d)\n", sem);

    int rc = Sys_SemFree(sem);
    assert(rc== P1_BLOCKED_PROCESSES);

    rc = Sys_SemV(sem);
    assert(rc== P1_SUCCESS);

    return 0;
}

static int
Blocks(void *arg)
{   
    int rc;
    int pid;
    int status;
    
    USLOSS_Console("Blocks: Running.\n");
    USLOSS_Console("Blocks: Forks Free\n");
    rc = Sys_Spawn("Free", Free, (void *) sem, USLOSS_MIN_STACK, 1, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Blocks: Calling P(%d)\n", sem);
    rc = Sys_SemP(sem);
    assert(rc==P1_SUCCESS);
    USLOSS_Console("Blocks quitting.\n");
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    assert(status == 0);

    return 0;
}

int P3_Startup(void *arg)
{
    int rc;
    int pid;
    int status;


    USLOSS_Console("startup: Creating 1 semaphore with initial value 0\n");
    rc = Sys_SemCreate("sem0", 0, &sem);
    assert(rc== P1_SUCCESS);

    USLOSS_Console("startup: Forking a process Blocks\n");
    rc = Sys_Spawn("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 1,  &pid);
    assert(rc==P1_SUCCESS);
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    assert(status == 0);
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

