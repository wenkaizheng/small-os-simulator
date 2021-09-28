#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>



void
startup(int argc, char **argv)
{
    P1SemInit();
    int rc;
    int sem =0 ;
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Sucessfully return %d.\n",sem);

    int rc1  ;
    int sem1 = 0;
    rc1 = P1_SemCreate("sem", 0, &sem1);
    assert(rc1 == P1_DUPLICATE_NAME);
    USLOSS_Console("Duplicate return %d.\n",sem1);

    
    int rc2  ;
    int sem2 =0;
    rc2 = P1_SemCreate("semmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm", 0, &sem2);;
    assert(rc2 == P1_NAME_TOO_LONG );
    USLOSS_Console("Name Too Long return %d.\n",sem2);

    int rc4 ;
    int sem4 =0;
    rc4 = P1_SemCreate(NULL, 0, &sem4);
    assert(rc4 == P1_NAME_IS_NULL );
    USLOSS_Console("Name is NULL return %d.\n",sem4);
    
    int i;
    for(i=0;i<1999;i++){
         
         int rc0 =0;
         int sem0;
         char name[10];
         sprintf(name, "%d", i);
         rc0 = P1_SemCreate(name, 0, &sem0);
         assert(rc0 == P1_SUCCESS );
         USLOSS_Console("Success many return %d.\n",sem0);
    }
    
    int rc3 ;
    int sem3 =0;
    rc3 = P1_SemCreate("hi", 0, &sem3);
    assert(rc3 ==  P1_TOO_MANY_SEMS );
    int j;
    assert(P1_SemFree(2000) == P1_INVALID_SID);
    for(j=0;j<1999;j++){
       assert( P1_SemFree(j) == P1_SUCCESS);
    }
    assert( P1_SemFree(0) == P1_INVALID_SID);
    int rc5 ;
    int sem5 =0 ;
    rc5 = P1_SemCreate("sem", 0, &sem5);
    assert(rc5 == P1_SUCCESS);
    USLOSS_Console("Sucessfully return %d.\n",sem5);
    
    USLOSS_Halt(0);
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}