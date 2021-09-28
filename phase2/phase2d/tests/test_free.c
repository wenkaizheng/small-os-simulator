/* 
 * Tests P1_SemFree() by freeing all the sems that have been created
 * by P1_SemCreate()
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
    int sems[P1_MAXSEM];


    int i;
    for (int k = 0; k < 10; k++) {
        for(i = 0; i < 1000; i++){
        	rc = Sys_SemCreate(MakeName("Sem", i), 0, &sems[i]);
        	assert(rc== P1_SUCCESS);
    	}
        USLOSS_Console("Freeing all the semaphores\n");
        for(i = 0; i < 1000; i++){
    	   rc = Sys_SemFree(sems[i]);
            assert(rc== P1_SUCCESS);
        }
    }
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

