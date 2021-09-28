#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

int cid, cid1, cid2;


static void Output(void *arg) {
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1ContextSwitch(cid);
}

static void helper(void *arg) {
    int rc, rc1, rc2;
    rc1 = P1ContextCreate(Output, "1\n", USLOSS_MIN_STACK, &cid1);
    assert(rc1 == P1_SUCCESS);
    rc2 = P1ContextCreate(Output, "1\n", USLOSS_MIN_STACK, &cid2);
    assert(rc2 == P1_SUCCESS);
    rc = P1ContextSwitch(cid1);
    assert(rc == P1_SUCCESS);


    rc = P1ContextFree(-1);
    assert(rc == P1_INVALID_CID);
    rc = P1ContextFree(50);
    assert(rc == P1_INVALID_CID);
    rc = P1ContextFree(20);
    assert(rc == P1_INVALID_CID);

    rc = P1ContextSwitch(cid2);
    assert(rc == P1_SUCCESS);

    rc = P1ContextFree(cid1);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cid1);
    assert(rc == P1_INVALID_CID);

    rc = P1ContextFree(cid2);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cid2);
    assert(rc == P1_INVALID_CID);



    USLOSS_Halt(0);
}


void
startup(int argc, char **argv) {
    int rc;
    P1ContextInit();
    rc = P1ContextCreate(helper, "1", USLOSS_MIN_STACK, &cid);
    assert(rc == P1_SUCCESS);
    rc = P1ContextSwitch(cid);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}