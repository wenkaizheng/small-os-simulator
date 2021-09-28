#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

static int Output2(void *arg) 
{
    char *msg = (char *) arg;

    USLOSS_Console("output2\n");
    USLOSS_Console("%s", msg);
    P1_ProcInfo pif;
    int pid = 1;
     USLOSS_Console("%d\n",P1_GetProcInfo(pid,&pif));
    if(pif.state == P1_STATE_RUNNING)
       USLOSS_Console("%s %d %d %d %d %d\n",pif.name, pif.sid,pif.priority,pif.tag,pif.cpu,pif.parent);
   
   
    //P1_Quit(11);
    P1_Quit(11);
    // should not return
    return 0;
}
static int
Output(void *arg) 
{
    char *msg = (char *) arg;
    
   
    USLOSS_Console("%s\n", msg);
    int rc;
    int pid;
    rc = P1_Fork("Hi", Output2, "Hello Worlds!\n", USLOSS_MIN_STACK, 2, 0, &pid);
    int pid1;
    int status;
    int result = P1GetChildStatus(0, &pid1, &status);
    assert(pid1 == 1);
    assert(status ==11);

    assert(result == P1_SUCCESS);  
  
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
    rc = P1_Fork("Hello", Output, "Hello World!\n", USLOSS_MIN_STACK, 3, 0, &pid);
 //   rc = P1_Fork("Hello45", Output, "Hello Worldss!\n", USLOSS_MIN_STACK, 2, 0, &pid);

    assert(rc == P1_DUPLICATE_NAME);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}