/*
 * Tests P1_SemFree() by passing invalid arguments
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
    USLOSS_Console("Creating 1 semaphore\n");
    rc = P1_SemCreate("sem0", 0, &sem);
    TEST(rc, P1_SUCCESS);
    
    USLOSS_Console("Double free.\n");
    rc = P1_SemFree(sem);
    TEST(rc, P1_SUCCESS);
    rc = P1_SemFree(sem);
    TEST(rc, P1_INVALID_SID);

    PASSED();
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
