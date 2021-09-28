#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "usloss.h"
#include "phase1.h"
#include "phase1Int.h"
#define enable(previous) \
    if (previous == 1)   \
        P1EnableInterrupts();
#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }
static void DeviceHandler(int type, void *arg);
static void SyscallHandler(int type, void *arg);
//static void IllegalInstructionHandler(int type, void *arg);

static int sentinel(void *arg);

static int devSems[8];   // self make
static int devStatus[8]; //self make
static int okToAbort;    // self make

void startup(int argc, char **argv)
{
    int pid, rc;
    P1SemInit();

    // initialize device data structures
    int i;
    char str[80];
    for (i = 0; i < 8; i++)
    {
        sprintf(str, "P1D_DEVSEMS_%d", i);
        rc = P1_SemCreate(str, 0, &(devSems[i]));
    }
    // put device interrupt handlers into interrupt vector
    USLOSS_IntVec[USLOSS_CLOCK_INT] = DeviceHandler;
    USLOSS_IntVec[USLOSS_ALARM_INT] = DeviceHandler;
    USLOSS_IntVec[USLOSS_DISK_INT] = DeviceHandler;
    USLOSS_IntVec[USLOSS_TERM_INT] = DeviceHandler;

    USLOSS_IntVec[USLOSS_MMU_INT] = NULL;
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;

    //USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalInstructionHandler;

    /* create the sentinel process */
    rc = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6, 0, &pid);
    // should not return
    assert(0);

} /* End of startup */
int checkTypeAndUnit(int type, int unit, int *semIndex)
{
    int result = P1_SUCCESS;
    if (type == USLOSS_CLOCK_DEV)
    {
        if (unit >= USLOSS_CLOCK_UNITS || unit < 0)
        {
            result = P1_INVALID_UNIT;
        }
        else
        {
            *semIndex = 0;
        }
    }
    else if (type == USLOSS_ALARM_DEV)
    {
        if (unit >= USLOSS_ALARM_UNITS || unit < 0)
        {
            result = P1_INVALID_UNIT;
        }
        else
        {
            *semIndex = 1;
        }
    }
    else if (type == USLOSS_DISK_DEV)
    {
        if (unit >= USLOSS_DISK_UNITS || unit < 0)
        {
            result = P1_INVALID_UNIT;
        }
        else
        {
            *semIndex = 2 + unit;
        }
    }
    else if (type == USLOSS_TERM_DEV)
    {
        if (unit >= USLOSS_TERM_UNITS || unit <0)
        {
            result = P1_INVALID_UNIT;
        }
        else
        {
            *semIndex = 4 + unit;
        }
    }
    else
    {
        result = P1_INVALID_TYPE;
    }
    return result;
}
int P1_WaitDevice(int type, int unit, int *status)
{
    // disable interrupts
    // check kernel mode
    mode_check();
    int previous = P1DisableInterrupts();
    int semIndex, rc;
    int result = checkTypeAndUnit(type, unit, &semIndex);
    //USLOSS_Console("P1_WaitDevice<123>: interrupt type %d\n", type);
    if (result == P1_SUCCESS)
    {
        // P device's semaphore
        rc = P1_P(devSems[semIndex]);
        if (!okToAbort)
        {
            // set *status to device's status
            *status = devStatus[semIndex];
        }
        else
        {
            result = P1_WAIT_ABORTED;
        }
    }
    // restore interrupts
    enable(previous);
    return result;
}

int P1_WakeupDevice(int type, int unit, int status, int abort)
{
    //USLOSS_Console("P1_WakeupDevice<145>: enter\n");
    mode_check();
    int previous = P1DisableInterrupts();
    int semIndex, rc;
    int result = checkTypeAndUnit(type, unit, &semIndex);
    // disable interrupts
    // check kernel mode
    // save device's status to be used by P1_WaitDevice
    if (result == P1_SUCCESS)
    {
        devStatus[semIndex] = status;
        // save abort to be used by P1_WaitDevice
        okToAbort = abort;
        // V device's semaphore
        rc = P1_V(devSems[semIndex]);
    }
    // restore interrupts
    enable(previous);
    return result;
}

static int ticks = 0;

static void
DeviceHandler(int type, void *arg)
{
    //USLOSS_Console("DeviceHandler<173>: enter\n");
    int unit = (int)arg;
    int status, rc;
    // if clock device
    if (type == USLOSS_CLOCK_DEV)
    {
        //USLOSS_Console("DeviceHandler<179>: %d\n", ticks);
        //      P1_WakeupDevice every 5 ticks
        ticks++;
        if (ticks % 5 == 0)
        {
            rc = USLOSS_DeviceInput((unsigned int)type, unit, &status);
            rc = P1_WakeupDevice(type, unit, status, 0);
        }
        //      P1Dispatch(TRUE) every 4 ticks
        if (ticks % 4 == 0)
        {
            P1Dispatch(1);
        }
        if(ticks == 20){
            ticks = 0;
        }
    }
    else
    {
        // else
        //      P1_WakeupDevice
        rc = USLOSS_DeviceInput((unsigned int)type, unit, &status);
        rc = P1_WakeupDevice(type, unit, status, 0);
    }
}

static int
sentinel(void *arg)
{
    int pid, cid;
    int rc;

    /* start the P2_Startup process */
    rc = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 2, 0, &pid);
    // enable interrupts
    P1EnableInterrupts();

    // while sentinel has children
    //      get children that have quit via P1GetChildStatus (either tag)
    //      wait for an interrupt via USLOSS_WaitInt
    P1_ProcInfo thisProcessInfo;
    int thisPid = P1_GetPid();
    rc = P1_GetProcInfo(thisPid, &thisProcessInfo);
    int status;

    assert(rc == P1_SUCCESS);
    while (thisProcessInfo.numChildren > 0)
    {
        rc = P1GetChildStatus(0, &cid, &status);
        rc = P1GetChildStatus(1, &cid, &status);
        USLOSS_WaitInt();
        rc = P1_GetProcInfo(thisPid, &thisProcessInfo);
    }
    USLOSS_Console("Sentinel quitting.\n");
    return 0;
} /* End of sentinel */

int P1_Join(int tag, int *pid, int *status)
{
    int result = P1_SUCCESS, rc, returnValue;
    // disable interrupt
    // kernel mode
    mode_check();
    int previous = P1DisableInterrupts();
    // do
    //     use P1GetChildStatus to get a child that has quit
    //     if no children have quit
    //        set state to P1_STATE_JOINING vi P1SetState
    //        P1Dispatch(FALSE)
    // until either a child quit or there are no more children
    while (1)
    {
        returnValue = P1GetChildStatus(tag, pid, status);
        if (returnValue == P1_SUCCESS || returnValue == P1_NO_CHILDREN || returnValue == P1_INVALID_TAG)
        {
            result = returnValue;
            break;
        }
        if (returnValue == P1_NO_QUIT)
        {
            //USLOSS_Console("P1_Join<267>: returnValue == P1_NO_QUIT\n");
            rc = P1SetState(P1_GetPid(), P1_STATE_JOINING, -1);
            P1Dispatch(0);
        }
    }
    enable(previous);
    return result;
}

static void
SyscallHandler(int type, void *arg)
{
    USLOSS_Console("System call %d not implemented.\n", (int)arg);
    USLOSS_IllegalInstruction();
}
void finish(int argc, char **argv) {}


//static void IllegalInstructionHandler(int type, void *arg)
//{
  //  USLOSS_IllegalInstruction();
//}