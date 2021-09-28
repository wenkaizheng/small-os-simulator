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
    char name[2*P1_MAXNAME];
    P1SemInit();

    USLOSS_Console("Creating 1 semaphore with a name of length > P1_MAXNAME\n");
    int i;
    for(i = 0; i < sizeof(name); i++) {
	   name[i] = 'a';
    }
    name[sizeof(name)-1] = '\0';
    rc = P1_SemCreate(name, 0, &sem);
    TEST(rc, P1_NAME_TOO_LONG);

    PASSED();
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
