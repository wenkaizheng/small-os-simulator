#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include  <stdlib.h>
static int Output2(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    int cpid ;
    int status;
    int result = P1GetChildStatus(0, &cpid, &status);
    assert(result==P1_NO_QUIT);
    P1_Quit(11);
    // should not return
    return 0;
}
static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    int pid;
    int rc = P1_Fork("Hellos", Output2, "Hello Worlds!\n", USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc ==P1_SUCCESS);
    int cpid ;
    int status;
    int result = P1GetChildStatus(0, &cpid, &status);
    assert(result==P1_SUCCESS);
    //USLOSS_Console("pppp");
    P1_Quit(11);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    /*
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");

    // fork test 
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 2, &pid);
    assert(rc == P1_INVALID_TAG );
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 0, 0, &pid);
    assert(rc == P1_INVALID_PRIORITY );
     rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK-1, 1, 0, &pid);
    assert(rc == P1_INVALID_STACK );
     char* name =NULL;
     rc = P1_Fork(name, Output, "Hello World!\n", USLOSS_MIN_STACK, 1, 0, &pid);
     assert(rc == P1_NAME_IS_NULL );
     rc = P1_Fork("Hi", Output, "Hello Worldssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss!\n"
     , USLOSS_MIN_STACK, 1, 0, &pid);
     assert(rc ==P1_NAME_TOO_LONG);

     // P1GetChildStatus test
     int cpid ;
     int status;
     assert(P1GetChildStatus(0, &cpid, &status) == P1_INVALID_TAG);

    // P1SetState test 
    assert(P1SetState(0, P1_STATE_RUNNING, 0) == P1_INVALID_STATE );
    assert(P1SetState(1, P1_STATE_JOINING, 0) == P1_INVALID_PID );
*/
    int pid;
    int rc;
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 3, 0, &pid);
    //int result = P1GetChildStatus(0, &cpid, &status);
    //assert(result==P1_NO_QUIT);

    P1_Quit(11);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}