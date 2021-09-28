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

    USLOSS_Console("Creating 1 semaphore with name=NULL\n");
    rc = Sys_SemCreate(NULL, 0, &sem);
    assert(rc==P1_NAME_IS_NULL);
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}
