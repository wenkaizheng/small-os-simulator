/*
 * Create multiple processes that wait on individual semaphores. Make sure the proper
 * process wakes up when its semaphore is V'ed.
 */

#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>

#include "tester.h"
#include "phase2Int.h"

#define NUMPROCS 10

static int sems[P1_MAXPROC];
static int pids[P1_MAXPROC];

static int 
Worker(void *arg)
{
    int i = (int) arg;
    int rc;

    // Wait on our semaphore.
    rc = Sys_SemP(sems[i]);
    assert(rc == P1_SUCCESS);
    int pid;
    Sys_GetPID(&pid);
    return  pid;
}

/*
 * Controller process that runs the show. Create the workers, V them randomly, and make
 * sure the correct worker quits each time.
 */

static int 
Controller(void *arg)
{
    int rc;
    int pid;
    int status;
    int i;

    for (i = 0; i < NUMPROCS; i++) {  
        rc = Sys_Spawn(MakeName("Worker", i), Worker, (void *) i, USLOSS_MIN_STACK, 3, &pids[i]);
        assert(rc == P1_SUCCESS);
    }
    int count = NUMPROCS;
    while(count > 0) {
        i = random() % count;
        rc = Sys_SemV(sems[i]);
        assert(rc == P1_SUCCESS);
      //  rc = P1GetChildStatus(0, &pid, &status);
        rc = Sys_Wait( &pid, &status);
        assert(rc == P1_SUCCESS);
        assert(pid == pids[i]);
        //USLOSS_Console("%d %d\n",status,pid);
        assert(status == pid);
        pids[i] = pids[count-1];
        sems[i] = sems[count-1];
        count--;
    }
   // PASSED();
    return 0;
}

int P3_Startup(void *arg) {
    int rc;
    int pid;

   // P1SemInit();
    for (int i = 0; i < NUMPROCS; i++) {
        rc = Sys_SemCreate(MakeName("Sem", i), 0, &sems[i]);
        assert(rc == P1_SUCCESS);
    }
    rc = Sys_Spawn("Controller", Controller, NULL, USLOSS_MIN_STACK, 5, &pid);
    assert(rc == P1_SUCCESS);    
    return 0;
}

void dummy(int type, void *arg) {};

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

