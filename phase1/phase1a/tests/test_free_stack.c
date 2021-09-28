#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <string.h>

/**
 * USLOSS ver 3.6
 *
 * There should only be 4,416 bytes in 8 blocks of leak caused by USLOSS
 *
 *
 */



static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    //USLOSS_Console("%s", msg);
    assert(strcmp(msg, "NetWorking")==0 || strcmp(msg, "Operation System")==0 );
    USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    int cid1, cid2;
    int rc1, rc2;
    P1ContextInit();
    rc1 = P1ContextCreate(Output, "NetWorking", USLOSS_MIN_STACK, &cid1);
    assert(0==cid1);
    assert(rc1 == P1_SUCCESS);
    
    rc2 = P1ContextCreate(Output, "Operation System", USLOSS_MIN_STACK, &cid2);
    assert(1==cid2);
    assert(rc2 == P1_SUCCESS);

    assert(P1ContextFree(cid1) == P1_SUCCESS);
    assert(P1ContextFree(cid2) == P1_SUCCESS);
    USLOSS_Halt(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}