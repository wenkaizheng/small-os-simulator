#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"
static int sem;

static int Output3(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("output3\n");
    USLOSS_Console("%s", msg);
    USLOSS_Console("%d\n",sem);
    assert(Sys_SemP(sem) ==  P1_SUCCESS);
    assert(Sys_SemP(sem) ==  P1_SUCCESS);
    assert(Sys_SemP(sem) ==  P1_SUCCESS);
    USLOSS_Console("output3 is going to quit\n");
    return 0;
}
static int Output2(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("output2\n");
    USLOSS_Console("%s", msg);
    int rc;
    int pid;
    rc = Sys_Spawn("nihao", Output3, "Hello Worldss!\n", USLOSS_MIN_STACK, 2, &pid);
    int pid1;
    int status;
    USLOSS_Console("%d\n",sem);
    assert(Sys_SemV(sem) ==  P1_SUCCESS);
    assert(Sys_SemV(sem) ==  P1_SUCCESS);
    int result = Sys_Wait(&pid1, &status);

    assert(result == P1_SUCCESS);  
    //P1_Quit(11);
    USLOSS_Console("output2 is going to quit\n");
  
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
    rc = Sys_Spawn("NDN", Output2, "Hello Worlds!\n", USLOSS_MIN_STACK, 2, &pid);
    int pid1;
    int status;
    assert(Sys_SemV(sem) ==0);
    int result = Sys_Wait (&pid1, &status);
    

    assert(result == P1_SUCCESS);  
     USLOSS_Console("here\n");
    USLOSS_Console("output is going to quit\n");
    return 0;
}

int P3_Startup(void *arg) 
{
    int pid;
    int rc;
   // P1SemInit();
    rc = Sys_SemCreate("sem", 0, &sem);
    assert(rc == P1_SUCCESS);
    USLOSS_Console("startup\n");
    rc = Sys_Spawn("NFD", Output, "Hello World!\n", USLOSS_MIN_STACK, 3, &pid);
    assert(rc == P1_SUCCESS);
    return 0;
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

