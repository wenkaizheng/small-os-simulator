#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"
static int sem;
static int
SemNameCheck(void *arg) 
{
    int rc;

    char name[P1_MAXNAME+1];

    rc = P1_SemName(sem, name);
    assert(strcmp("sem",name)==0);
    assert(rc == P1_SUCCESS);
    return 0;
}

int P3_Startup(void *arg) 
{
    int rc;
    int pid;
    
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);

    // Checks Semaphore name
    rc = Sys_Spawn("SemNameCheck", SemNameCheck, (void *) sem, USLOSS_MIN_STACK, 1, &pid);
    assert(rc == P1_SUCCESS);
    return 0;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

