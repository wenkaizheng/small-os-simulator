/*
 * Tests freeing a semaphore that has a process blocked on it
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include "tester.h"

static int sem;

static int 
Free(void *arg){
    USLOSS_Console("Free: Running.\n");
    USLOSS_Console("Free: Calling SemFree(%d)\n", sem);

    int rc = P1_SemFree(sem);
    TEST(rc, P1_BLOCKED_PROCESSES);
    PASSED();
    return 0;
}

static int
Blocks(void *arg)
{   
    int rc;
    int pid;
    
    USLOSS_Console("Blocks: Running.\n");
    USLOSS_Console("Blocks: Forks Free\n");
    rc = P1_Fork("Free", Free, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Blocks: Calling P(%d)\n", sem);
    rc = P1_P(sem);
    TEST(rc, P1_SUCCESS);
    USLOSS_Console("Blocks quitting.\n");
    
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

    USLOSS_Console("startup: Forking a process Blocks\n");
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    TEST(rc, P1_SUCCESS);
    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
