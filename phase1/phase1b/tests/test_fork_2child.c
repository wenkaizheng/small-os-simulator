#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static int
Output2(void *arg)
{
    char *msg = (char *) arg;


    USLOSS_Console("Output startup\n");
    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    return 0;
}




static int
Output(void *arg) 
{


    int pid;
    int rc;
    USLOSS_Console("Output startup\n");
    rc = P1_Fork("Hello2", Output2, "Hello World2!\n", USLOSS_MIN_STACK, 1, 0,
            &pid);
    assert(rc == P1_SUCCESS);
    int pid1;
    int status;
    int result = P1GetChildStatus(0, &pid1, &status);

    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    P1_Quit(11);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 3, 0,
            &pid);

    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}