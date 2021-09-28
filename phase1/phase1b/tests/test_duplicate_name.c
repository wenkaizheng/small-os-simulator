#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include  <stdlib.h>
static int
Output2(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("%s", msg);
    //USLOSS_Console("pppp");
  
    return 0;
}
static int
Output(void *arg) 
{
    char *msg = (char *) arg;

    //USLOSS_Console("pppp");
    int pid; 
    int rc ;
    rc = P1_Fork("Hello", Output2, "Hello World2!\n", USLOSS_MIN_STACK, 1, 0,
            &pid);
    assert(rc ==  P1_DUPLICATE_NAME);
    USLOSS_Console("%s", msg);
    int cpid;
    int status;
    assert(P1GetChildStatus(0,&cpid,&status) == P1_NO_QUIT);
    USLOSS_Console("abc");
    P1_Quit(11);
    // should not return
    return 0;
}
void
startup(int argc, char **argv)
{
    

    // duplicate name

    int pid; 
    int rc ;
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 2, 0, &pid);
   
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}