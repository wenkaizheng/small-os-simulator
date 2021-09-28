/*
 * Tests all Phase1c functions
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tester.h"

static int sem;

static int Proc(void *arg){
    int i = (int) arg;
    int rc;

    char name[P1_MAXNAME+1];
    USLOSS_Console("Proc%d: Running.\n", i);
    rc = P1_SemName(sem, name);
    TEST(rc, P1_SUCCESS);
    TEST(strcmp(name, "sem0"), 0);

    if(i != 4){
        USLOSS_Console("Proc%d: Calling P(%d)\n", i, sem);
        rc = P1_P(sem);
        TEST(rc, P1_SUCCESS);
    } else {
        USLOSS_Console("Proc%d: Calling V(%d) to unblock the blocked processes\n", i, sem);
        for(int j = 0; j < 4; j++){
            rc = P1_V(sem);
            TEST(rc, P1_SUCCESS);
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

    USLOSS_Console("Parent: Running.\n");
    USLOSS_Console("Parent: Forking 4 processes Proc1, Proc2, Proc3, and Proc4 with priority 1\n");
    for (int i = 1; i < 5; i++) {
        rc = P1_Fork(MakeName("Proc", i), Proc, (void *) i, USLOSS_MIN_STACK, 1, 0, &pid);
        assert(rc == P1_SUCCESS);
    }

    USLOSS_Console("Parent: Freeing the sem.\n");
    rc = P1_SemFree(sem);
    TEST(rc, P1_SUCCESS);
    USLOSS_Console("Parent: Quitting.\n");
    PASSED();
    // should not return
    assert(0);
    return 0;
}

void
startup(int argc, char **argv)
{
    int rc;
    int pid;

    P1SemInit();
    USLOSS_Console("startup: Creating 1 semaphore with initial value 0\n");
    rc = P1_SemCreate("sem0", 0, &sem);
    TEST(rc, P1_SUCCESS);

    USLOSS_Console("startup: Forking a process Parent with priority 2\n");
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc == P1_SUCCESS);    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
