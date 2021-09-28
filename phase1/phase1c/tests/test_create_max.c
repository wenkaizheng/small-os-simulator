/*
 * Create the maximum number of semaphores and not one more.
 */
#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>
#include "tester.h"

void
startup(int argc, char **argv)
{
    int rc;
    int sem;

    P1SemInit();
    int i;
    USLOSS_Console("Creating P1_MAXSEM Semaphores\n");
    for(i = 0; i < P1_MAXSEM; i++){
        rc = P1_SemCreate(MakeName("Sem", i), 0, &sem);
        TEST(rc, P1_SUCCESS);
	}
    // Next one should fail
    USLOSS_Console("The next one should fail.\n");
    rc = P1_SemCreate(MakeName("Sem", i), 0, &sem);
    TEST(rc, P1_TOO_MANY_SEMS);
    PASSED();
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
