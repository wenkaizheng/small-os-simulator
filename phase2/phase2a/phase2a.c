#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <stdio.h>
#include <assert.h>
#include <libuser.h>
#include <usyscall.h>
#include "phase2Int.h"

#define TAG_KERNEL 0
#define TAG_USER 1
#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }

static void SpawnStub(USLOSS_Sysargs *sysargs);

static void WaitStub(USLOSS_Sysargs *sysargs);

static void TerminateStub(USLOSS_Sysargs *sysargs);

static void ProcInfoStub(USLOSS_Sysargs *sysargs);

static void GetPIDStub(USLOSS_Sysargs *sysargs);

static void GetTimeOfDayStub(USLOSS_Sysargs *sysargs);

//to be valid the number should be in the range (0, USLOSS_MAX_SYSCALLS] 
static void (*syscallHandlers[USLOSS_MAX_SYSCALLS + 1 ])(USLOSS_Sysargs *args);


//we should not enable and disable interrupt
//we should not use mutex semaphores unless there is shared data structure
//we need to do mode check

/*
 * IllegalHandler
 *
 * Handler for illegal instruction interrupts.
 *
 */
static void
IllegalHandler(int type, void *arg) {
    mode_check()
    P1_ProcInfo info;
    assert(type == USLOSS_ILLEGAL_INT);

    int pid = P1_GetPid();
    int rc = P1_GetProcInfo(pid, &info);
    assert(rc == P1_SUCCESS);
    if (info.tag == TAG_KERNEL) {
        P1_Quit(1024);
    } else {
        P2_Terminate(2048);
    }
}

/*
 * SyscallHandler
 *
 * Handler for system call interrupts.
 *
 */
static void
SyscallHandler(int type, void *arg) {
    mode_check()
    assert(type == USLOSS_SYSCALL_INT);
    int number = ((USLOSS_Sysargs *) arg)->number;
    if (syscallHandlers[number] != NULL) {
        (*syscallHandlers[number])((USLOSS_Sysargs *) arg);
    } else {
        ((USLOSS_Sysargs *) arg)->arg4 = (void *)P2_INVALID_SYSCALL;
    }
}

/*
 * P2ProcInit
 *
 * Initialize everything.
 *
 */
void P2ProcInit(void) {
    mode_check()
    int rc;

    USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalHandler;
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = SyscallHandler;

    // call P2_SetSyscallHandler to set handlers for all system calls
    // todo do we need to set up just these function call？that's enough
    int i;
    for (i = 0; i < USLOSS_MAX_SYSCALLS + 1; ++i) {
        syscallHandlers[i] = NULL;
    }
    rc = P2_SetSyscallHandler(SYS_SPAWN, SpawnStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_WAIT, WaitStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_TERMINATE, TerminateStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_GETPROCINFO, ProcInfoStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_GETPID, GetPIDStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_GETTIMEOFDAY, GetTimeOfDayStub);
    assert(rc == P1_SUCCESS);
}

/*
 * P2_SetSyscallHandler
 *
 * Set the system call handler for the specified system call.
 *
 */
int P2_SetSyscallHandler(unsigned int number,
                         void (*handler)(USLOSS_Sysargs *args)) {
    mode_check()
    //to be valid the number should be in the range (0, USLOSS_MAX_SYSCALLS]
    if (number <= 0 || number > USLOSS_MAX_SYSCALLS) {
        return P2_INVALID_SYSCALL;
    }
    syscallHandlers[number] = handler;
    return P1_SUCCESS;
}

// self make wrapper
// self make struct for saving args and function
typedef struct wrapperStruct {
    void *arg;

    int (*func)(void *arg);

} wrapperStruct;

static int wrapper(void *arg) {
    int (*func)(void *arg) = ((wrapperStruct *) arg)->func;
    void *funcArg = ((wrapperStruct *) arg)->arg;
    free((wrapperStruct *) arg);
    int rc = USLOSS_PsrSet(USLOSS_PsrGet() & ~0x1);
    assert(rc == USLOSS_DEV_OK);
    int status = func(funcArg);
    Sys_Terminate(status);
    assert(0);
    return 0;
}

/*
 * P2_Spawn
 *
 * Spawn a user-level process.
 *
 */
int P2_Spawn(char *name, int (*func)(void *arg), void *arg, int stackSize,
             int priority, int *pid) {
    // todo what if name is null or it is too long or duplicate name no problem already
    mode_check()
    wrapperStruct *wrapperArg = malloc(sizeof(wrapperStruct));
    wrapperArg->arg = arg;
    wrapperArg->func = func;
    int rc =P1_Fork(name, wrapper, wrapperArg, stackSize, priority, TAG_USER,
                    pid);
    if (rc!=P1_SUCCESS){
        free(wrapperArg);
    }
    return rc;
}

/*
 * P2_Wait
 *
 * Wait for a user-level process.
 *
 */
int P2_Wait(int *pid, int *status) {
    mode_check()
    // todo not sure do we need to more stuff for this function
    int rc = P1_Join(TAG_USER, pid, status);
    return rc;
}

/*
 * P2_Terminate
 *
 * Terminate a user-level process.
 *
 */
void P2_Terminate(int status) {
    mode_check()
    // todo not sure do we need to more stuff for this function
    P1_Quit(status);
}

/*
 * SpawnStub
 *
 * Stub for Sys_Spawn system call. 
 *
 */
static void
SpawnStub(USLOSS_Sysargs *sysargs) {
    mode_check()
    int (*func)(void *) = sysargs->arg1;
    void *arg = sysargs->arg2;
    int stackSize = (int) sysargs->arg3;
    int priority = (int) sysargs->arg4;
    char *name = sysargs->arg5;
    int pid;
    int rc = P2_Spawn(name, func, arg, stackSize, priority, &pid);
    if (rc == P1_SUCCESS) {
        sysargs->arg1 = (void *) pid;
    }
    sysargs->arg4 = (void *) rc;
}

//self made

static void
WaitStub(USLOSS_Sysargs *sysargs) {
    mode_check()
    int pid;
    int status;
    int returnCode;
    returnCode = P2_Wait(&pid, &status);
    sysargs->arg1 = (void *) pid;
    sysargs->arg2 = (void *) status;
    sysargs->arg4 = (void *) returnCode;
}

static void
TerminateStub(USLOSS_Sysargs *sysargs) {
    mode_check()
    int status = (int) sysargs->arg1;
    P2_Terminate(status);
}

static void
ProcInfoStub(USLOSS_Sysargs *sysargs) {
    mode_check()
    int pid = (int) sysargs->arg1;
    P1_ProcInfo *info = (P1_ProcInfo *) sysargs->arg2;
    sysargs->arg4 = (void *) P1_GetProcInfo(pid, info);
}

static void
GetPIDStub(USLOSS_Sysargs *sysargs) {
    mode_check()
    sysargs->arg1 = (void *) P1_GetPid();
}

static void
GetTimeOfDayStub(USLOSS_Sysargs *sysargs) {
    mode_check()
    int rc = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, (int *) &(sysargs->arg1));
    assert(rc == USLOSS_DEV_OK);
}