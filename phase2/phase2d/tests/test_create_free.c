#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>
#include <libdisk.h>

#include "tester.h"
#include "phase2Int.h"


int P3_Startup(void *arg)
{
    int rc;
    int sem =0 ;
    rc = Sys_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Sucessfully return %d.\n",sem);

    int rc1  ;
    int sem1 = 0;
    rc1 = Sys_SemCreate("sem", 0, &sem1);
    assert(rc1 == P1_DUPLICATE_NAME);
    USLOSS_Console("Duplicate return %d.\n",sem1);

    
    int rc2  ;
    int sem2 =0;
    rc2 = Sys_SemCreate("semmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm", 0, &sem2);;
    assert(rc2 == P1_NAME_TOO_LONG );
    USLOSS_Console("Name Too Long return %d.\n",sem2);

    int rc4 ;
    int sem4 =0;
    rc4 = Sys_SemCreate(NULL, 0, &sem4);
    assert(rc4 == P1_NAME_IS_NULL );
    USLOSS_Console("Name is NULL return %d.\n",sem4);
    
    int i;
    int sems[3000];
    for(i=500;i<1900;i++){

         int rc0 =0;
         char name[10];
         sprintf(name, "%d", i);
         rc0 = Sys_SemCreate(name, 0, &(sems[i]));
         assert(rc0 == P1_SUCCESS );
         USLOSS_Console("Success many return %d.\n",sems[i]);
    }

    int j;
    assert(Sys_SemFree(2000) == P1_INVALID_SID);
    for(j=500;j<1900;j++){
       assert( Sys_SemFree(sems[j]) == P1_SUCCESS);
    }
    return 1;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}