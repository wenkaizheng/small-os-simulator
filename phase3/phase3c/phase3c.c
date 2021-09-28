/*
 * phase3c.c
 *
 */
/*
   Info:
   Group Member: Wenkai Zheng & Jiacheng Yang
   NetId:        wenkaizheng & jiachengyang
   Submission type: Group
*/
#include <assert.h>
#include <phase1.h>
#include <phase2.h>
#include <usloss.h>
#include <string.h>
#include <libuser.h>

#include "phase3.h"
#include "phase3Int.h"

#ifdef DEBUG
int debugging3 = 1;
#else
int debugging3 = 0;
#endif

void debug3(char *fmt, ...) {
    va_list ap;

    if (debugging3) {
        va_start(ap, fmt);
        USLOSS_VConsole(fmt, ap);
    }
}

#define P3C_ABORT_OPERATION -97

#define P(semId, rc)  \
    rc = P1_P(semId); \
    assert(rc == P1_SUCCESS);

#define V(semId, rc)  \
    rc = P1_V(semId); \
    assert(rc == P1_SUCCESS);

#define semCreate(name, value, semId, rc)  \
    rc = P1_SemCreate(name, value, semId); \
    assert(rc == P1_SUCCESS);

#define semFree(semId, rc)  \
    rc = P1_SemFree(semId); \
    assert(rc == P1_SUCCESS);

// This allows the skeleton code to compile. Remove it in your solution.
#define UNUSED __attribute__((unused))

static int Pager(void *arg);

typedef struct Frame {
    int isBeingUsed;
    int mappedProcessId;
} Frame;


Frame *framePool = NULL;
int framePoolSize, numberOfPagesForOneProcess;
int vmMutex, framePoolMutex;
int isP3PagerInit = FALSE;

/*
 *----------------------------------------------------------------------
 *
 * P3FrameInit --
 *
 *  Initializes the frame data structures.
 *
 * Results:
 *   P3_ALREADY_INITIALIZED:    this function has already been called
 *   P1_SUCCESS:                success
 *
 *----------------------------------------------------------------------
 */
int P3FrameInit(int pages, int frames) {
    CheckMode()

    // initialize the frame data structures, e.g. the pool of free frames
    // set P3_vmStats.freeFrames
    if (framePool != NULL) {
        return P3_ALREADY_INITIALIZED;
    }
    int rc;
    semCreate("Phase3c_vmStats_Mutex", 1, &vmMutex, rc)
    semCreate("Phase3c_framePool_Mutex", 1, &framePoolMutex, rc)
    numberOfPagesForOneProcess = pages;
    framePoolSize = frames;
    framePool = malloc(sizeof(Frame) * frames);
    for (int i = 0; i < framePoolSize; ++i) {
        (framePool + i)->isBeingUsed = 0;
        (framePool + i)->mappedProcessId = -1;
    }
    P(vmMutex, rc)
    P3_vmStats.freeFrames = frames;
    V(vmMutex, rc)
    P3_PrintStats(&P3_vmStats);
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3FrameShutdown --
 *
 *  Cleans up the frame data structures.
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3FrameInit has not been called
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */
int P3FrameShutdown(void) {
    CheckMode()
    int result = P1_SUCCESS;
    // clean things up
    if (framePool == NULL) {
        return P3_NOT_INITIALIZED;
    }

    int rc;
    // should P3_vmStats.freeFrames be zero？
    // john said it does not have to be 0
    //P(vmMutex, rc)
    //P3_vmStats.freeFrames = 0;
    //V(vmMutex, rc)


    //free frame pool
    free(framePool);
    framePool = NULL;
    framePoolSize = 0;


    // free two mutex
    semFree(vmMutex, rc)
    semFree(framePoolMutex, rc)
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * P3FrameFreeAll --
 *
 *  Frees all frames used by a process
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3FrameInit has not been called
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */

int P3FrameFreeAll(int pid) {
    CheckMode()

    if (framePool == NULL) {
        return P3_NOT_INITIALIZED;
    }
    // free all frames in use by the process (P3PageTableGet)
    USLOSS_PTE *table = NULL;
    int rc = P3PageTableGet(pid, &table);
    int usedFrameCounter = 0;
    assert(rc == P1_SUCCESS);

    P(framePoolMutex, rc)
    for (int i = 0; i < numberOfPagesForOneProcess; ++i) {
        //suggested by wenkai
        if ((table + i)->incore == (unsigned int) 1) {
            ++usedFrameCounter;
            framePool[(table + i)->frame].isBeingUsed = 0;
            framePool[(table + i)->frame].mappedProcessId = -1;
            (table + i)->write = 0;
            (table + i)->read = 0;
            (table + i)->incore = 0;
            (table + i)->frame = 0;
        }
    }
    V(framePoolMutex, rc)
    P(vmMutex, rc)
    P3_vmStats.freeFrames += usedFrameCounter;
    V(vmMutex, rc)
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3FrameMap --
 *
 *  Maps a frame to an unused page and returns a pointer to it.
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3FrameInit has not been called
 *   P3_OUT_OF_PAGES:       process has no free pages
 *   P3_INVALID_FRAME       the frame number is invalid
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */
int P3FrameMap(int frame, void **ptr) {
    CheckMode()
    if (framePool == NULL) {
        return P3_NOT_INITIALIZED;
    }
    if (frame < 0 || frame >= framePoolSize) {
        return P3_INVALID_FRAME;
    }
    // get the page table for the process (P3PageTableGet)
    USLOSS_PTE *table = NULL;
    int pid = P1_GetPid();
    int rc = P3PageTableGet(pid, &table);
    assert(rc == P1_SUCCESS);
    int i;
    // find an unused page
    for (i = 0; i < numberOfPagesForOneProcess; ++i) {
        if ((table + i)->incore == (unsigned int) 0) {
            break;
        }
    }
    if (i == numberOfPagesForOneProcess) {
        return P3_OUT_OF_PAGES;
    }
    // update the page's PTE to map the page to the frame
    (table + i)->incore = 1;
    (table + i)->read = 1;
    (table + i)->write = 1;
    (table + i)->frame = (unsigned int)frame;
    P(framePoolMutex, rc)
    //framePool[frame].isBeingUsed = 1;
    framePool[frame].mappedProcessId = pid;
    V(framePoolMutex, rc)
    // update the page table in the MMU (USLOSS_MmuSetPageTable)
    rc = USLOSS_MmuSetPageTable(table);
    assert(rc == USLOSS_MMU_OK);

    char *vmRegion;
    int numPagerPtr;
    int pageSize = USLOSS_MmuPageSize();
    vmRegion = USLOSS_MmuRegion(&numPagerPtr);
    *ptr = vmRegion + (pageSize * i);
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3FrameUnmap --
 *
 *  Opposite of P3FrameMap. The frame is unmapped.
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3FrameInit has not been called
 *   P3_FRAME_NOT_MAPPED:   process didn’t map frame via P3FrameMap
 *   P3_INVALID_FRAME       the frame number is invalid
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */
int P3FrameUnmap(int frame) {
    CheckMode()

    // get the process's page table (P3PageTableGet)
    if (framePool == NULL) {
        return P3_NOT_INITIALIZED;
    }
    if (frame < 0 || frame >= framePoolSize) {
        return P3_INVALID_FRAME;
    }

    // get the page table for the process (P3PageTableGet)
    USLOSS_PTE *table = NULL;
    int pid = P1_GetPid();
    int rc = P3PageTableGet(pid, &table);
    assert(rc == P1_SUCCESS);

    // verify that the process mapped the frame
    P(framePoolMutex, rc)
    if (framePool[frame].mappedProcessId != pid) {
        V(framePoolMutex, rc)
        return P3_FRAME_NOT_MAPPED;
    }

    framePool[frame].mappedProcessId = -1;
    V(framePoolMutex, rc)
    int i;
    for (i = 0; i < numberOfPagesForOneProcess; ++i) {
        if ((table + i)->incore == (unsigned int) 1 &&
            (unsigned int) frame == (table + i)->frame) {
            break;
        }
    }

    // update page's PTE to remove the mapping
    (table + i)->incore = 0;
    (table + i)->frame = 0;
    (table + i)->read = 0;
    (table + i)->write = 0;
    // update the page table in the MMU (USLOSS_MmuSetPageTable)
    rc = USLOSS_MmuSetPageTable(table);
    assert(rc == USLOSS_MMU_OK);
    return P1_SUCCESS;
}

// information about a fault. Add to this as necessary.

typedef struct Fault {
    PID pid;
    int offset;
    int cause;
    SID wait;
    // other stuff goes here
    int okToSetPTE;
    int status;
    USLOSS_PTE *table;
} Fault;

typedef struct pageFaultQueueNode {
    Fault *fault;
    struct pageFaultQueueNode *next;
} pageFaultQueueNode;

static int pageFaultQueueMutex;
static int isPageFaultQueueReady;
static pageFaultQueueNode *pageFaultQueueHead = NULL;
static pageFaultQueueNode *pageFaultQueueTail = NULL;
static int numberOfPagers;

/**
 * insert into end by using tail
 * the first time is all node
 * returns whether this insert works or not
 * */
int pageFaultQueueInsert(Fault *fault) {
    CheckMode()
    if (fault == NULL) {
        return FALSE;
    }
    int rc, result = TRUE;
    P(pageFaultQueueMutex, rc)
    pageFaultQueueNode **head = &(pageFaultQueueHead);
    pageFaultQueueNode **tail = &(pageFaultQueueTail);
    pageFaultQueueNode *node = malloc(sizeof(pageFaultQueueNode));
    node->next = NULL;
    node->fault = fault;
    if ((*head == NULL && *tail != NULL) || (*head != NULL && *tail == NULL)) {
        result = FALSE;
    } else if (*head == NULL && *tail == NULL) {
        *head = node;
        *tail = node;
    } else {
        (*tail)->next = node;
        *tail = node;
    }
    V(pageFaultQueueMutex, rc)
    V(isPageFaultQueueReady, rc)
    return result;
}

/**
 * remove top node from queue
 * and give the fault to **fault for using
 * */
int pageFaultQueueRemove(Fault **fault) {
    CheckMode()
    if (fault == NULL) {
        return FALSE;
    }
    int rc, result = TRUE;
    P(isPageFaultQueueReady, rc)
    P(pageFaultQueueMutex, rc)
    pageFaultQueueNode **head = &(pageFaultQueueHead);
    pageFaultQueueNode **tail = &(pageFaultQueueTail);
    if ((*head == NULL && *tail == NULL) || (*head == NULL && *tail != NULL) ||
        (*head != NULL && *tail == NULL)) {
        result = FALSE;
    } else if (*head == *tail) { // just one node in
        pageFaultQueueNode *copy = *head;
        *head = NULL;
        *tail = NULL;
        *fault = copy->fault;
        free(copy);
    } else { // more than one
        pageFaultQueueNode *copy = *head;
        *head = (*head)->next;
        *fault = copy->fault;
        free(copy);
    }
    V(pageFaultQueueMutex, rc)
    return result;
}

/**
 * free all node in the queue
 * and make sure head and tail need to be NULL
 * */
void freeAllRequestQueue() {
    CheckMode()
    int rc;
    P(pageFaultQueueMutex, rc)
    pageFaultQueueNode **head = &(pageFaultQueueHead);
    pageFaultQueueNode **tail = &(pageFaultQueueTail);
    if (*head != NULL && *tail != NULL) {
        pageFaultQueueNode *walker = *head;
        pageFaultQueueNode *copy;
        while (walker != NULL) {
            copy = walker;
            walker = walker->next;
            free(copy);
        }
        *head = NULL;
        *tail = NULL;
    }
    V(pageFaultQueueMutex, rc)
}

/*
 *----------------------------------------------------------------------
 *
 * FaultHandler --
 *
 *  Page fault interrupt handler
 *
 *----------------------------------------------------------------------
 */
// if pagers are not initialized; print a error
// the type should be correct
static void
FaultHandler(int type, void *arg) {
    CheckMode()
    assert(type == USLOSS_MMU_INT);
    char buffer[P1_MAXNAME + 1];
    int rc;
    Fault fault UNUSED;

    fault.offset = (int) arg;
    // fill in other fields in fault
    fault.pid = P1_GetPid();
    fault.cause = USLOSS_MmuGetCause();

    rc = P3PageTableGet(fault.pid, &(fault.table));

    assert(rc == P1_SUCCESS);
    sprintf(buffer, "P3C_PAGE_FAULT_%d", fault.pid);
    semCreate(buffer, 0, &(fault.wait), rc)

    // add to queue of pending faults
    // let pagers know there is a pending fault
    pageFaultQueueInsert(&fault);
    // wait for fault to be handled
    P(fault.wait, rc);
    semFree(fault.wait, rc);
    if (fault.okToSetPTE == FALSE) {
       P2_Terminate(fault.status);
    } else {
        rc = USLOSS_MmuSetPageTable(fault.table);
        assert(rc == USLOSS_MMU_OK);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * P3PagerInit --
 *
 *  Initializes the pagers.
 *
 * Results:
 *   P3_ALREADY_INITIALIZED: this function has already been called
 *   P3_INVALID_NUM_PAGERS:  the number of pagers is invalid
 *   P1_SUCCESS:             success
 *
 *----------------------------------------------------------------------
 */
int P3PagerInit(int pages, int frames, int pagers) {
    CheckMode()

    if (isP3PagerInit == TRUE) {
        return P3_ALREADY_INITIALIZED;
    }
    if (pagers > P3_MAX_PAGERS || pagers < 1) {
        return P3_INVALID_NUM_PAGERS;
    }
    USLOSS_IntVec[USLOSS_MMU_INT] = FaultHandler;

    int rc;
    numberOfPagers = pagers;
    // initialize the pager data structures
    semCreate("PHASE3C_PageFaultQueue_MUTEX", 1, &pageFaultQueueMutex, rc)

    semCreate("PHASE3C_PageFaultQueue_READY_SEMAPHORE", 0,
              &isPageFaultQueueReady, rc)
    // fork off the pagers and wait for them to start running
    int pid;
    char buffer[P1_MAXNAME + 1];
    for (int i = 0; i < pagers; ++i) {
        sprintf(buffer, "P3C_PAGERS_%d", i);
        rc = P1_Fork(buffer, Pager, NULL, 2 * USLOSS_MIN_STACK,
                     P3_PAGER_PRIORITY, 0, &pid);
        assert(rc == P1_SUCCESS);
    }

    isP3PagerInit = TRUE;
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3PagerShutdown --
 *
 *  Kills the pagers and cleans up.
 *
 * Results:
 *   P3_NOT_INITIALIZED:     P3PagerInit has not been called
 *   P1_SUCCESS:             success
 *
 *----------------------------------------------------------------------
 */
int P3PagerShutdown(void) {
    CheckMode()
    if (isP3PagerInit != TRUE) {
        return P3_NOT_INITIALIZED;
    }
    char buffer[P1_MAXNAME + 1];
    int rc;
    int pid, status;

    // cause the pagers to quit
    freeAllRequestQueue();
    for (int i = 0; i < numberOfPagers; ++i) {
        Fault fault;
        fault.cause = P3C_ABORT_OPERATION;
        sprintf(buffer, "P3C_PAGERS_KILLER_%d", i);
        semCreate(buffer, 0, &fault.wait, rc)
        pageFaultQueueInsert(&fault);
        P(fault.wait, rc)
        semFree(fault.wait, rc);
        rc = P1_Join(0, &pid, &status);
        assert(rc==P1_SUCCESS);
    }
    // clean up the pager data structures
    freeAllRequestQueue();
    semFree(pageFaultQueueMutex, rc)
    semFree(isPageFaultQueueReady, rc)

    isP3PagerInit = FALSE;
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * Pager --
 *
 *  Handles page faults
 *
 *----------------------------------------------------------------------
 */

static int
Pager(void *arg) {
    CheckMode()
    /********************************
notify P3PagerInit that we are running
loop until P3PagerShutdown is called
    wait for a fault
    if it's an access fault kill the faulting process
    if there are free frames
        frame = a free frame
    else
        P3SwapOut(&frame);
    rc = P3SwapIn(pid, page, frame)
    if rc == P3_EMPTY_PAGE
        P3FrameMap(frame, &addr)
        zero-out frame at addr
        P3FrameUnmap(frame);
    else if rc == P3_OUT_OF_SWAP
        kill the faulting process
    update PTE in faulting process's page table to map page to frame
    unblock faulting process
**********************************/

    Fault *fault = NULL;
    int rc1, rc2;
    while (1) {
        pageFaultQueueRemove(&fault);
        if (fault->cause == P3C_ABORT_OPERATION) {
            V(fault->wait, rc1)
            break;
        }

        P(vmMutex, rc1)
        P3_vmStats.faults++;
        V(vmMutex, rc1)

        if (fault->cause == USLOSS_MMU_ACCESS) {
            fault->okToSetPTE = FALSE;
            fault->status = USLOSS_MMU_ACCESS;
            V(fault->wait, rc1)
            continue;
        }
        int page = (fault->offset) / USLOSS_MmuPageSize();
        int frame, OkToUpdateVMState;
        P(framePoolMutex, rc1)
        // todo can be optimized
        for (frame = 0; frame < framePoolSize; ++frame) {
            if (framePool[frame].isBeingUsed == 0) {
                break;
            }
        }
        if (frame < framePoolSize) {
            framePool[frame].isBeingUsed = 1;
            OkToUpdateVMState= TRUE;
        } else {
            rc2 = P3SwapOut(&frame);
            OkToUpdateVMState= FALSE;
        }

        V(framePoolMutex, rc1)
        rc2 = P3SwapIn(fault->pid, page, frame);

        char *addr;
        if (rc2 == P3_EMPTY_PAGE) {
            rc1 = P3FrameMap(frame, (void **)&addr);
            assert(P1_SUCCESS == rc1);
            int frameSize = USLOSS_MmuPageSize();
            for (int i = 0; i < frameSize; ++i) {
                addr[i] = '\0';
            }
            rc1 = P3FrameUnmap(frame);
            assert(P1_SUCCESS == rc1);
        } else if (rc2 == P3_OUT_OF_SWAP) {
            fault->okToSetPTE = FALSE;
            fault->status = P3_OUT_OF_SWAP;
            V(fault->wait, rc1)
            continue;
        }
        if(OkToUpdateVMState) {
            P(vmMutex, rc1)
            // todo if we did not pick a free frame from the frame pool, but
            //  instead swap out a frame. In this situation, do we need to update
            //  freeFrames?
            P3_vmStats.freeFrames--;
            P3_vmStats.new++;
            V(vmMutex, rc1)
        }
        fault->okToSetPTE = TRUE;
        ((fault->table) + page)->frame = (unsigned int) frame;
        ((fault->table) + page)->incore = 1;
        ((fault->table) + page)->read = 1;
        ((fault->table) + page)->write = 1;
        V(fault->wait, rc1)
    }
    return 0;
}