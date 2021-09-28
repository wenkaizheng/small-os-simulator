/*
 * Tests P1_SemCreate() with invalid arguments
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

int P3_Startup(void *arg)
{
    int rc;
    int sem;
    char name[2*P1_MAXNAME];

    USLOSS_Console("Creating 1 semaphore with a name of length > P1_MAXNAME\n");
    int i;
    for(i = 0; i < sizeof(name); i++) {
	   name[i] = 'a';
    }
    name[sizeof(name)-1] = '\0';
    rc = Sys_SemCreate(name, 0, &sem);
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

