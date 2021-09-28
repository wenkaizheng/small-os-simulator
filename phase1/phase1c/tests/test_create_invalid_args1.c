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

    USLOSS_Console("Creating 1 semaphore with name=NULL\n");
    rc = P1_SemCreate(NULL, 0, &sem);
    TEST(rc, P1_NAME_IS_NULL);
    
    PASSED();
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
