#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>
#include <libdisk.h>

#include "tester.h"
#include "phase2Int.h"

static int passed = FALSE;


#define MSG "This is a test."
typedef struct diskRequestInfo {
    void *arg1, *arg2, *arg3, *arg4;
    int sem;
} diskRequestInfo;

typedef struct diskRequestQueueNode {
    diskRequestInfo *info;
    struct diskRequestQueueNode *next;
} diskRequestQueueNode;

extern  int diskRequestQueueInsert(diskRequestInfo *info, int unit) CHECKRETURN;
extern  int diskRequestQueueRemove(diskRequestInfo **info, int unit) CHECKRETURN;
extern void freeAllRequestQueue(int unit);

int tryToGet(void *arg){
    int unit = (int) arg;
    int value;
    diskRequestInfo *info;
    int rc = diskRequestQueueRemove(&info, unit);
    assert(rc == 1);
    value = info->sem;
    return value;
}


int queueInsertAndDelete1(void *arg)
{
    DumpProcesses();
    int rc;
    int i;
    int unit = (int) arg;
    for ( i = 0; i < 50; i++)
    {
        USLOSS_Console("40th\n");
        diskRequestInfo *info = malloc(sizeof(diskRequestInfo));
        info->sem = i;
        rc = diskRequestQueueInsert(info, unit);
        assert(rc ==1);
    }
    int pid, waitPid, status;
    char name[20];
    sprintf(name, "tryToGet%d", unit);
    diskRequestInfo *info = malloc(sizeof(diskRequestInfo));
    info->sem = 1232412412+unit;
    freeAllRequestQueue(unit);
    rc = P1_Fork(name, tryToGet, arg, 4 * USLOSS_MIN_STACK, 2, 0, &pid);
    assert(rc== P1_SUCCESS);
    rc = diskRequestQueueInsert(info, unit);
    assert(rc ==1);
    rc = P1_Join(0,&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(waitPid== pid);
    assert(status== 1232412412+unit);
    return 11;
}


int P2_Startup(void *arg)
{
    int rc, waitPid, status , pid1, pid2;

    P2ClockInit();
    P2DiskInit();
    P2DiskShutdown();
    assert(P2_Sleep(5)==P1_SUCCESS);
    rc = P1_Fork("queueInsertAndDelete1", queueInsertAndDelete1, 0,
                 4 * USLOSS_MIN_STACK, 2, 0, &pid1);
    assert(rc== P1_SUCCESS);

    rc = P1_Fork("queueInsertAndDelete2", queueInsertAndDelete1, (void *)1,
                 4 * USLOSS_MIN_STACK, 2, 0, &pid2);
    assert(rc== P1_SUCCESS);


    DumpProcesses();
    rc = P1_Join(0,&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(status== 11);

    rc = P1_Join(0,&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(status== 11);


    P2ClockShutdown();
    USLOSS_Console("You passed all the tests! Treat yourself to a cookie!\n");
    PASSED();
    return 0;
}

void test_setup(int argc, char **argv)
{
}

void test_cleanup(int argc, char **argv)
{
    USLOSS_Console("ENTER");
    DeleteAllDisks();
    if (passed)
    {
        USLOSS_Console("TEST PASSED.\n");
    }
}