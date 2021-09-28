#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdio.h>

static void
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    USLOSS_Halt(0);
}

void
startup(int argc, char **argv)
{
    int cids[51];
    int rcs[51];
    char str[10];
    P1ContextInit();

    int i;
    for (i = 0; i < 51; ++i) {
        sprintf(str, "%d", i);
        rcs[i] = P1ContextCreate(Output, str, USLOSS_MIN_STACK,&cids[i]);
        if(i==50){

            assert(rcs[i] == P1_TOO_MANY_CONTEXTS);
        } else {
            assert(rcs[i] == P1_SUCCESS);
        }
    }

    for (i = 0; i < 50; ++i) {
        P1ContextFree(cids[i]);
    }

    USLOSS_Halt(0);

    // should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}