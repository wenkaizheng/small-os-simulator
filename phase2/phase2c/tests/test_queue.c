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

int queueInsertAndDelete1(void *arg)
{
    DumpProcesses();
    int rc;
    int i;
    for ( i = 0; i < 50; i++)
    {
        USLOSS_Console("40th\n");
        diskRequestInfo *info = malloc(sizeof(diskRequestInfo));
        info->sem = i;
        rc = diskRequestQueueInsert(info, 0);
        assert(rc ==1);
    }
    for ( i = 0; i < 50; i++)
    {
        diskRequestInfo *info;
        assert(P2_Sleep(10)==0);
        rc = diskRequestQueueRemove(&info, 0);
        assert(rc == 1);
        assert(info->sem ==i);
    }
    return 11;
}


int queueInsertAndDelete2(void *arg)
{
    DumpProcesses();
    USLOSS_Console("58th\n");
    int i;
    for ( i = 0; i < 50; i++)
    {
        USLOSS_Console("61th\n");
        diskRequestInfo *info = malloc(sizeof(diskRequestInfo));
        info->sem = i;
        int rc = diskRequestQueueInsert(info, 0);
        assert(rc ==1);
        USLOSS_Console("%d\n",i);
    }
    USLOSS_Console("65th\n");
    for ( i = 0; i < 50; i++)
    {
        USLOSS_Console("74th\n");
        
        diskRequestInfo *info;
        int rc = diskRequestQueueRemove(&info, 0);
        assert(rc == 1);
        assert(info->sem ==i);
    }
    return 11;
}
int P2_Startup(void *arg)
{
    int rc, waitPid, status, p3Pid , p4Pid;

    P2ClockInit();
    P2DiskInit();
    P2DiskShutdown();
    assert(P2_Sleep(5)==P1_SUCCESS);
    rc = P1_Fork("P3_Startup", queueInsertAndDelete2, NULL,
            4 * USLOSS_MIN_STACK, 2, 0,&p3Pid);
    assert(rc== P1_SUCCESS);
    USLOSS_Console("82th\n");
    
    rc = P1_Fork("queueInsertAndDelete1", queueInsertAndDelete1, NULL,
                 4 * USLOSS_MIN_STACK, 2, 0, &p4Pid);
    assert(rc== P1_SUCCESS);
    USLOSS_Console("85th\n");
    

    DumpProcesses();
    rc = P1_Join(0,&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(waitPid== p3Pid);
    USLOSS_Console("90th\n");
    assert(status== 11);
    
    rc = P1_Join(0,&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(waitPid== p4Pid);
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