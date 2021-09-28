/*
 * Tests P1_SemCreate() with invalid arguments
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

    USLOSS_Console("Creating 2 semaphores with the same name\n");
    rc = Sys_SemCreate("sem0", 0, &sem);
    assert(rc==P1_SUCCESS);

    rc = Sys_SemCreate("sem0", 0, &sem);
    assert(rc==P1_DUPLICATE_NAME);
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

