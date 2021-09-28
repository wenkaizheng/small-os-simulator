/*
 * Tests P1_SemFree() by passing invalid arguments
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

int P3_Startup(void *arg)
{
    int rc;
    int sem;

    USLOSS_Console("Creating 1 semaphore\n");
    rc = Sys_SemCreate("sem0", 0, &sem);
    assert(rc==P1_SUCCESS);
    
    USLOSS_Console("Double free.\n");
    rc = Sys_SemFree(sem);
    assert(rc==P1_SUCCESS);
    rc = Sys_SemFree(sem);
    assert(rc== P1_INVALID_SID);

    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

