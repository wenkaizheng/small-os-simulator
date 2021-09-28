/* 
 * Tests P1_SemFree() by freeing all the sems that have been created
 * by P1_SemCreate()
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
    int sems[P1_MAXSEM];

    P1SemInit();
    int i;
    for (int k = 0; k < 10; k++) {
        USLOSS_Console("Creating P1_MAXSEM semaphores\n");
        for(i = 0; i < P1_MAXSEM; i++){
        	rc = P1_SemCreate(MakeName("Sem", i), 0, &sems[i]);
        	TEST(rc, P1_SUCCESS);
    	}
        USLOSS_Console("Freeing all the semaphores\n");
        for(i = 0; i < P1_MAXSEM; i++){
    	   rc = P1_SemFree(sems[i]);
    	   TEST(rc, P1_SUCCESS);
        }
    }
    PASSED();
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
