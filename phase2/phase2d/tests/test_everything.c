/*
 * Tests all Phase1c functions
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

static int Proc(void *arg){
    int i = (int) arg;
    int rc;

    char name[P1_MAXNAME+1];
    USLOSS_Console("Proc%d: Running.\n", i);
    rc = Sys_SemName(sem, name);
    assert(rc==P1_SUCCESS);
    assert(strcmp(name, "sem0")== 0);

    if(i != 4){
        USLOSS_Console("Proc%d: Calling P(%d)\n", i, sem);
        rc = Sys_SemP(sem);
        assert(rc== P1_SUCCESS);
    } else {
        USLOSS_Console("Proc%d: Calling V(%d) to unblock the blocked processes\n", i, sem);
        for(int j = 0; j < 4; j++){
            rc = Sys_SemV(sem);
            assert(rc==P1_SUCCESS);
        }
    }
    USLOSS_Console("Proc%d: Work done. Quitting.\n", i);
    return 12;
    }


static int
Blocks(void *arg)
{   
    int rc;
    int pid;
    int status;

    USLOSS_Console("Parent: Running.\n");
    USLOSS_Console("Parent: Forking 4 processes Proc1, Proc2, Proc3, and Proc4 with priority 1\n");
    int i;
    for ( i = 1; i < 5; i++) {
        rc = Sys_Spawn(MakeName("Proc", i), Proc, (void *) i, USLOSS_MIN_STACK, 1,
                &pid);
        assert(rc == P1_SUCCESS);
    }

    for ( i = 1; i < 5; i++) {
        rc = Sys_Wait(&pid, &status);
        assert(rc == P1_SUCCESS);
        assert(status == 12);
    }

    USLOSS_Console("Parent: Freeing the sem.\n");
    rc = Sys_SemFree(sem);
    assert(rc== P1_SUCCESS);
    USLOSS_Console("Parent: Quitting.\n");
    return 1;
}

int P3_Startup(void *arg)
{
    int rc;
    int pid;
    int status;

    USLOSS_Console("startup: Creating 1 semaphore with initial value 0\n");
    rc = Sys_SemCreate("sem0", 0, &sem);
    assert(rc== P1_SUCCESS);
    USLOSS_Console("startup: Forking a process Parent with priority 2\n");
    rc = P2_Spawn("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 2,  &pid);
    assert(rc == P1_SUCCESS);
    rc = Sys_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    assert(status == 1);
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

