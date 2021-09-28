/*
 * Tests P1_SemCreate() with invalid arguments
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

    USLOSS_Console("Creating 2 semaphores with the same name\n");
    rc = P1_SemCreate("sem0", 0, &sem);
    TEST(rc, P1_SUCCESS);

    rc = P1_SemCreate("sem0", 0, &sem);
    TEST(rc, P1_DUPLICATE_NAME);
    
    PASSED();
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
