#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
static int sem;

static int Output3(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("output3\n");
    USLOSS_Console("%s", msg);
    assert(P1_P(5) == P1_INVALID_SID);
    assert(P1_P(-1) == P1_INVALID_SID);
    assert(P1_P(2000) == P1_INVALID_SID);
    USLOSS_Console("%d\n",sem);
    assert(P1_P(sem) ==  P1_SUCCESS);
    assert(P1_P(sem) ==  P1_SUCCESS);
    assert(P1_P(sem) ==  P1_SUCCESS);
    USLOSS_Console("output3 is going to quit\n");
    //P1_Quit(11);
    P1_Quit(11);
    // should not return
    return 0;
}
static int Output2(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("output2\n");
    USLOSS_Console("%s", msg);
    int rc;
    int pid;
    rc = P1_Fork("nihao", Output3, "Hello Worldss!\n", USLOSS_MIN_STACK, 2, 0, &pid);
    int pid1;
    int status;
    assert(P1_V(5) == P1_INVALID_SID);
    assert(P1_V(-1) == P1_INVALID_SID);
    assert(P1_V(2000) == P1_INVALID_SID);
    USLOSS_Console("%d\n",sem);
    assert(P1_V(sem) ==  P1_SUCCESS);
    assert(P1_V(sem) ==  P1_SUCCESS);
    int result = P1GetChildStatus(0, &pid1, &status);

    assert(result == P1_NO_QUIT);  
    //P1_Quit(11);
    USLOSS_Console("output2 is going to quit\n");
    P1_Quit(11);
    // should not return
    return 0;
}
static int
Output(void *arg) 
{
    char *msg = (char *) arg;
    
    USLOSS_Console("output\n");
    USLOSS_Console("%s", msg);
    int rc;
    int pid;
    rc = P1_Fork("NDN", Output2, "Hello Worlds!\n", USLOSS_MIN_STACK, 2, 0, &pid);
    int pid1;
    int status;
    assert(P1_V(sem) ==0);
    int result = P1GetChildStatus(0, &pid1, &status);
    int result2 = P1GetChildStatus(0, &pid1, &status);

    assert(result == P1_SUCCESS);  
    assert(result2 == P1_SUCCESS); 
    USLOSS_Console("here\n");
     USLOSS_Console("output is going to quit\n");
    P1_Quit(11);
    // should not return
    return 0;
}

void
startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1SemInit();
    rc = P1_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("startup\n");
    rc = P1_Fork("NFD", Output, "Hello World!\n", USLOSS_MIN_STACK, 3, 0, &pid);
   
    assert(rc == P1_DUPLICATE_NAME);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}