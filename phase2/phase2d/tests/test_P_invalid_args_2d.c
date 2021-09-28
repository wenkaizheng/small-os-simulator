/*
 * Tests calling P1_P() with invalid sids 
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

static int
Worker(void *arg)
{   
    int rc;
    assert((int)arg == 74);
    USLOSS_Console("Worker: Running.\n");

    USLOSS_Console("Worker: Calling P(-1)\n");
    rc = P1_P(-1);
    assert(rc ==P1_INVALID_SID);
    return 0;
}

int P3_Startup(void *arg)
{
    int rc;
    int sem;
    int pid;


    USLOSS_Console("startup: Creating 1 semaphore with initial value 0\n");
    rc = Sys_SemCreate("sem0", 0, &sem);
    assert(rc ==P1_SUCCESS);

    USLOSS_Console("startup: Forking a process Worker\n");
    rc = Sys_Spawn("Worker", Worker, (void *) 74, USLOSS_MIN_STACK, 1, &pid);
    assert(rc == P1_SUCCESS);
    
    return 0;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}
