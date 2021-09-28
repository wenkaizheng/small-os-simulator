/*
   Info:
   Group Member: Wenkai Zheng Jiacheng Yang
   NetId:        wenkaizheng & jiachengyang
   Submission type: Group       
 */
#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>
#include <libdisk.h>

#include "phase2Int.h"

static void     CreateStub(USLOSS_Sysargs *sysargs);
static void     PStub(USLOSS_Sysargs *sysargs);
static void     VStub(USLOSS_Sysargs *sysargs);
static void     FreeStub(USLOSS_Sysargs *sysargs);
static void     NameStub(USLOSS_Sysargs *sysargs);

/*
 * I left this useful function here for you to use for debugging. If you add -DDEBUG to CFLAGS
 * this will produce output, otherwise it won't. The message is printed including the function
 * and line from which it was called. I don't remember while it is called "debug2".
 */
#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }
static void debug2(char *fmt, ...)
{
    #ifdef DEBUG
    va_list ap;
    va_start(ap, fmt);
    USLOSS_Console("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);   \
    USLOSS_VConsole(fmt, ap);
    #endif
}

int P2_Startup(void *arg)
{
    int rc, pid;

    // initialize clock and disk drivers
    P2ClockInit();
    P2DiskInit();

    debug2("starting\n");
    rc = P2_SetSyscallHandler(SYS_SEMCREATE, CreateStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_SEMP, PStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_SEMV, VStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_SEMFREE, FreeStub);
    assert(rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_SEMNAME, NameStub);
    assert(rc == P1_SUCCESS);
    // ...
    rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4*USLOSS_MIN_STACK, 3, &pid);
    assert(rc == P1_SUCCESS);
    // ...

    // shut down clock and disk drivers
    int status;
    rc = P2_Wait(&pid, &status);
    assert(rc == P1_SUCCESS);
    P2ClockShutdown();
    P2DiskShutdown();

    return 0;
}

static void
CreateStub(USLOSS_Sysargs *sysargs)
{
    sysargs->arg4 = (void *) P1_SemCreate((char *) sysargs->arg2, (int) sysargs->arg1, 
                                          (void *) &sysargs->arg1);
}
static void PStub(USLOSS_Sysargs *sysargs)
{
    mode_check();
    int rc = P1_P((int)sysargs->arg1);
    sysargs->arg4 = (void *)rc;
}

static void VStub(USLOSS_Sysargs *sysargs)
{
    mode_check();
    int rc = P1_V((int)sysargs->arg1);
    sysargs->arg4 = (void *)rc;
}
static void FreeStub(USLOSS_Sysargs *sysargs)
{
    mode_check();
    int rc = P1_SemFree((int)sysargs->arg1);
    sysargs->arg4 = (void *)rc;
}
static void NameStub(USLOSS_Sysargs *sysargs)
{
    mode_check();
    int rc = P1_SemName((int)sysargs->arg1, (char *)sysargs->arg2);
    sysargs->arg4 = (void *)rc;
}
