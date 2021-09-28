/*
 * Tests calling P1_P() 3 times on a sem with initial value = 3
 */

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include "tester.h"

static int sem;


static int
Blocks(void *arg)
{   
    int rc;
    
    USLOSS_Console("Blocks: Running.\n");

    USLOSS_Console("Blocks: Calling P(%d) 3 times\n", sem);
    int i;
    for(i = 0; i < 3; i++){
    	rc = P1_P(sem);
    	TEST(rc, P1_SUCCESS);
    }
    USLOSS_Console("Blocks quitting.\n");
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
    USLOSS_Console("startup: Creating 1 semaphore with initial value 3\n");
    rc = P1_SemCreate("sem0", 3, &sem);
    TEST(rc, P1_SUCCESS);

    USLOSS_Console("startup: Forking a process Blocks\n");
    rc = P1_Fork("Blocks", Blocks, (void *) sem, USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    
    assert(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
