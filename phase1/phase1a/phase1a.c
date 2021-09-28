#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define enable(previous) \
    if (previous == 1)   \
        P1EnableInterrupts();
#define mode_check() \
    if( (USLOSS_PsrGet()& USLOSS_PSR_CURRENT_MODE)!=1){\
         USLOSS_Console("You are in the user mode");\
         USLOSS_Halt(1);\
    }
extern USLOSS_PTE *P3_AllocatePageTable(int pid);

extern void P3_FreePageTable(int pid);

typedef struct Context {
    void (*startFunc)(void *);

    void *startArg;
    USLOSS_Context context;
    // you'll need more stuff here
    int isBeingUsed; // self-added
    char *stack;
} Context;

static Context contexts[P1_MAXPROC];
// change cid to CID
//static int cid = -1;
static int CID = -1;

/*
 * Helper function to call func passed to P1ContextCreate with its arg.
 */
static void launch(void) {
    assert(contexts[CID].startFunc != NULL);
    contexts[CID].startFunc(contexts[CID].startArg);
}

void P1ContextInit(void) {
    // initialize contexts
    // change to kernel mode
    int status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_MODE);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    mode_check();
    int i;
    for (i = 0; i < P1_MAXPROC; i++) {
        contexts[i].isBeingUsed = 0;
    }
}

int P1ContextCreate(void (*func)(void *), void *arg, int stacksize, int *cid) {
    // turnoff interrupt self-made
    // self-strategy
    //USLOSS_Console("%d\n", USLOSS_PsrGet()& USLOSS_PSR_CURRENT_MODE);
    mode_check();
    int previous = P1DisableInterrupts();

    // stack size checking
    if (stacksize < USLOSS_MIN_STACK) {
        enable(previous);
        return P1_INVALID_STACK;
    }

    // check available contexts with getting id
    int id;
    for (id = 0; id < P1_MAXPROC; id++) {
        if (contexts[id].isBeingUsed == 0)
            break;
    }
    if (id == P1_MAXPROC) {
        enable(previous);
        return P1_TOO_MANY_CONTEXTS;
    }

    int result = P1_SUCCESS;
    // find a free context and initialize it
    // allocate the stack, specify the startFunc, etc.
    *cid = id;
    USLOSS_PTE *PTE = P3_AllocatePageTable(id);

    contexts[id].startArg = arg;
    contexts[id].startFunc = func;
    contexts[id].stack = malloc(sizeof(char) * stacksize);
    contexts[id].isBeingUsed = 1;
    USLOSS_ContextInit(&(contexts[id].context), contexts[id].stack,
                       sizeof(char) * stacksize, PTE, launch);
    enable(previous);
    return result;
}

int P1ContextSwitch(int cid) {
    mode_check();
    int previous = P1DisableInterrupts();
    int result = P1_SUCCESS;
    // switch to the specified context
    if (cid < 0 || cid >= P1_MAXPROC || contexts[cid].isBeingUsed == 0) {
        result = P1_INVALID_CID;
    } else {
        if (CID == -1) {
            CID = cid;
            USLOSS_ContextSwitch(NULL, &(contexts[cid].context));
        } else {
            int previousCID = CID;
            CID = cid;
            USLOSS_ContextSwitch(&(contexts[previousCID].context),
                                 &(contexts[CID].context));
        }
    }
    enable(previous);
    return result;
}

int P1ContextFree(int cid) {
    mode_check();
    // 0 means is disable 1 is enable
    int previous = P1DisableInterrupts();
    int result = P1_SUCCESS;
    //invalid
    if (cid < 0 || cid >= P1_MAXPROC || contexts[cid].isBeingUsed == 0) {
        result = P1_INVALID_CID;
    } else { //valid
        // free the stack and mark the context as unused
        P3_FreePageTable(cid);
        contexts[cid].isBeingUsed = 0;
        free(contexts[cid].stack);
    }
    enable(previous);
    return result;
}

void P1EnableInterrupts(void) {
    // clear the interrupt bit in the PSR
    // set to 1
    mode_check();
    int status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
}

/*
 * Returns true if interrupts were enabled, false otherwise.
 */
int P1DisableInterrupts(void) {
    mode_check();
    int enabled = (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_INT) >> 1;
    // set enabled to TRUE if interrupts are already enabled
    // set the interrupt bit in the PSR
    // set to 0;
    int status = USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);
    if (status != USLOSS_ERR_OK) {
        USLOSS_Halt(status);
    }
    return enabled;
}