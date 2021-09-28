#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
static void
Output(void *arg)
{
    char *msg = (char *)arg;

    USLOSS_Console("%s", msg);
    USLOSS_Halt(0);
}
static void
Outputs(void *arg)
{
    char *msg = (char *)arg;

    USLOSS_Console("%s", msg);
    USLOSS_Halt(0);
}
void startup(int argc, char **argv)
{
    // child
    if (fork() == 0)
    {
        sleep(1);
        int cid;
        int rc;
        P1ContextInit();
        rc = P1ContextCreate(Output, "Hello World!\n", USLOSS_MIN_STACK, &cid);
        assert(rc == P1_SUCCESS);
        USLOSS_Console("%d\n", cid);
        rc = P1ContextSwitch(cid);
        // should not return
        assert(rc == P1_SUCCESS);
        assert(0);
    }
    // parent
    else
    {
        int status;
        wait(&status);
        int cid;
        int rc;
        P1ContextInit();
        rc = P1ContextCreate(Outputs, "Hello Worlds!\n", USLOSS_MIN_STACK, &cid);
        assert(rc == P1_SUCCESS);
        USLOSS_Console("%d\n", cid);
        rc = P1ContextSwitch(cid);
        // should not return
        assert(rc == P1_SUCCESS);
        assert(0);

    }
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}
