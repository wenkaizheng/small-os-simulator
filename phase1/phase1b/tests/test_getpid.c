#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
static int Output3(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("output3\n");
    USLOSS_Console("%s", msg);
    assert(P1_GetPid()==1);
    //P1_Quit(11);
    P1_Quit(11);
    // should not return
    return 0;
}
static int
Output(void *arg) 
{
    char *msg = (char *) arg;
    int pid;
    int rc;
    USLOSS_Console("%s", msg);
    rc = P1_Fork("Hellos", Output3, "Hello Worlds!\n", USLOSS_MIN_STACK, 1, 0, &pid);
    assert(rc == P1_SUCCESS);
    int pid1;
    int status;
    int result = P1GetChildStatus(0, &pid1, &status);

    assert(result == P1_SUCCESS); 
    USLOSS_Console("%d\n",P1_GetPid()); 
    assert(P1_GetPid()==0);
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
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 2, 0, &pid);
  //  assert(rc == P1_SUCCESS);
    //assert(P1_GetPid()==0);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}