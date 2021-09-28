/*
 * Tests V on semaphore before P.
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

static int flag = 0;

static int
Unblocks(void *arg)
{
    int sem = (int) arg;
    int rc;

    flag = 1;
    USLOSS_Console("V on semaphore.\n");
    rc = Sys_SemV(sem);
    assert (rc== P1_SUCCESS);
    return 12;
}

static int
Blocks(void *arg) 
{
    int sem = (int) arg;
    int rc;
    int pid;

    rc = Sys_Spawn("Unblocks", Unblocks, (void *) sem, USLOSS_MIN_STACK, 1,&pid);
    assert(rc == P1_SUCCESS);

    USLOSS_Console("P on semaphore.\n");
    rc = Sys_SemP(sem);
    assert(rc == P1_SUCCESS);
    assert(flag== 1);
    return 0;
}


int P3_Startup(void *arg)
{
    int pid;
    int rc;
    int sem;

   // P1SemInit();
    rc = Sys_SemCreate("sem", 0, &sem);
    assert(rc== P1_SUCCESS);
    char buffer[P1_MAXNAME+1];
    rc = Sys_SemName(sem, buffer);
    assert(rc== P1_SUCCESS);
    assert(strncmp("sem", buffer, sizeof(buffer)) == 0);
    // Unblocks V's semaphore before Block P's
    rc = Sys_Spawn("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 2, &pid);
    assert(rc == P1_SUCCESS);
    return 0;
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

