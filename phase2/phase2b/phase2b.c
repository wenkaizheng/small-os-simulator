#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <usloss.h>
#include <phase1.h>

#include "phase2Int.h"
#define TAG_KERNEL 0
#define TAG_USER 1
#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }
static int ClockDriver(void *);
static void SleepStub(USLOSS_Sysargs *sysargs);

typedef struct sleepingProcessInfo
{
    int pid;
    int startTime;
    int restTime;
    int isBeingUsed;
    int semId;
} sleepingProcessInfo;

static sleepingProcessInfo SPInfoArray[P1_MAXPROC];

static int P2B_SPInfoArray_MutexSemId;
// todo model check

/*
 * P2ClockInit
 *
 * Initialize the clock data structures and fork the clock driver.
 */
void P2ClockInit(void)
{
    mode_check();
    int rc;

    P2ProcInit();

    // initialize data structures here
    int i = 0;
    char str[40];
    for (; i < P1_MAXPROC; i++)
    {
        SPInfoArray[i].isBeingUsed = 0;
        sprintf(str, "P2B_SPInfoArray_semaphore_%d", i);
        rc = P1_SemCreate(str, 0, &(SPInfoArray[i].semId));
        assert(rc == P1_SUCCESS);
    }
    rc = P1_SemCreate("P2B_SPInfoArray_Mutex_Semaphore", 1, &P2B_SPInfoArray_MutexSemId);
    assert(rc == P1_SUCCESS);

    rc = P2_SetSyscallHandler(SYS_SLEEP, SleepStub);
    assert(rc == P1_SUCCESS);

    // fork the clock driver here
    int pid;
    rc = P1_Fork("phase2b_clock_driver", ClockDriver, 0, USLOSS_MIN_STACK, 2,
                 TAG_KERNEL, &pid);
    assert(rc == P1_SUCCESS);
}

/*
 * P2ClockShutdown
 *
 * Clean up the clock data structures and stop the clock driver.
 */

//todo when will this func be called
void P2ClockShutdown(void)
{
   mode_check();
    // stop clock driver
   int rc = P1_WakeupDevice(USLOSS_CLOCK_DEV, 0, -1, 1); // todo do we need a semaphore here
    assert(rc == P1_SUCCESS);
    // todo clean up the clock data structures
    rc = P1_P(P2B_SPInfoArray_MutexSemId);
    assert(rc == P1_SUCCESS);
    int i=0;
    for(;i<P1_MAXPROC;i++){
        SPInfoArray[i].isBeingUsed=0;
        rc = P1_SemFree(SPInfoArray[i].semId);
        assert(rc == P1_SUCCESS);
    }
    rc = P1_SemFree(P2B_SPInfoArray_MutexSemId);
    assert(rc == P1_SUCCESS);
    
}

/*
 * ClockDriver
 *
 * Kernel process that manages the clock device and wakes sleeping processes.
 */
static int
ClockDriver(void *arg)
{
    mode_check();
    while (1)
    {
        int rc;
        int now;

        // wait for the next interrupt
        rc = P1_WaitDevice(USLOSS_CLOCK_DEV, 0, &now);
        if (rc == P1_WAIT_ABORTED)
        {
            break;
        }
        assert(rc == P1_SUCCESS);

        // wakeup any sleeping processes whose wakeup time has arrived
        int i = 0;
        // todo problem
        rc = P1_P(P2B_SPInfoArray_MutexSemId);
        assert(rc == P1_SUCCESS);
        for (; i < P1_MAXPROC; i++)
        {

            if (SPInfoArray[i].isBeingUsed == 1 && (SPInfoArray[i].restTime + SPInfoArray[i].startTime) <= now)
            {
                SPInfoArray[i].isBeingUsed = 0;
                rc = P1_V(SPInfoArray[i].semId);
                assert(rc == P1_SUCCESS);
            }
        }
        rc = P1_V(P2B_SPInfoArray_MutexSemId);
        assert(rc == P1_SUCCESS);
    }
    return P1_SUCCESS;
}

/*
 * P2_Sleep
 *
 * Causes the current process to sleep for the specified number of seconds.
 */
int P2_Sleep(int seconds)
{
    mode_check();
    if (seconds < 0)
    {
        return P2_INVALID_SECONDS;
    }
    // todo do we need to sleep if seconds is zero?
    // todo if P2ClockShutdown is called and has freed the array, what should we do.
    // add current process to data structure of sleepers
    // wait until sleep is complete
    if (seconds > 0)
    {
        int pid = P1_GetPid();
        int  rc = P1_P(P2B_SPInfoArray_MutexSemId);
        assert(rc == P1_SUCCESS);
        SPInfoArray[pid].pid = pid;
        rc = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, &(SPInfoArray[pid].startTime));
        assert(rc == USLOSS_DEV_OK);
        SPInfoArray[pid].restTime = seconds*1000000;
        SPInfoArray[pid].isBeingUsed = 1;
        rc = P1_V(P2B_SPInfoArray_MutexSemId);
        assert(rc == P1_SUCCESS);
        rc = P1_P(SPInfoArray[pid].semId);
        assert(rc == P1_SUCCESS);
    }
    return P1_SUCCESS;
}

/*
 * SleepStub
 *
 * Stub for the Sys_Sleep system call.
 */
static void
SleepStub(USLOSS_Sysargs *sysargs)
{
    mode_check();
    int seconds = (int)sysargs->arg1;
    int rc = P2_Sleep(seconds);
    sysargs->arg4 = (void *)rc;
}