/*
   Info:
   Group Member: Wenkai Zheng & Jiacheng Yang
   NetId:        wenkaizheng & jiachengyang
   Submission type: Group
*/
/*
 * phase3d.c
 *
 */

/***************
NOTES ON SYNCHRONIZATION
There are various shared resources that require proper synchronization. 
Swap space. Free swap space is a shared resource, we don't want multiple pagers choosing the
same free space to hold a page. You'll need a mutex around the free swap space.
The clock hand is also a shared resource.
The frames are a shared resource in that we don't want multiple pagers to choose the same frame via
the clock algorithm. That's the purpose of marking a frame as "busy" in the pseudo-code below. 
Pagers ignore busy frames when running the clock algorithm.
A process's page table is a shared resource with the pager. The process changes its page table
when it quits, and a pager changes the page table when it selects one of the process's pages
in the clock algorithm. 
Normally the pagers would perform I/O concurrently, which means they would release the mutex
while performing disk I/O. I made it simpler by having the pagers hold the mutex while they perform
disk I/O.
***************/

#include <assert.h>
#include <phase1.h>
#include <phase2.h>
#include <usloss.h>
#include <string.h>
#include <libuser.h>

#include "phase3.h"
#include "phase3Int.h"

#ifdef DEBUG
static int debugging3 = 1;
#else
static int debugging3 = 0;
#endif
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

static void debug3(char *fmt, ...) {
    va_list ap;

    if (debugging3) {
        va_start(ap, fmt);
        USLOSS_VConsole(fmt, ap);
    }
}

// todo update vm
typedef struct {
    int pid;
    int page;
    int frame;
    int track;
    int first;
} Block;

typedef struct {
    int isBusy;
    int blockNum;
} Frame;

static int PAGES, FRAMES;
static int sector, track, disk;
static int isP3SwapInit = FALSE;
static int sizeOfFrame, numberOfSectorsForAFrame;
static int numberOfBlocks, numberOfFreeBlocks;
static Block *blockPool;
static int blockPoolMutex, vmStateMutex;
static Frame *framesInfoArr;
static int hand = -1;


static void frameInfo() {
    debug3("-------------------frame info--------------------\n");
    for (int i = 0; i < FRAMES; ++i) {
        debug3("frame %d isBusy %d blockNum %d\n", i, framesInfoArr[i].isBusy,
               framesInfoArr[i].blockNum);
    }
    debug3("-------------------------------------------------\n\n");
}

static void blockInfo(int all) {
    debug3("-------------------block info--------------------\n");
    for (int i = 0; i < numberOfBlocks; ++i) {
        if (all != 1 && blockPool[i].pid == -1) {
            continue;
        }
        debug3("block %3d pid %2d page %3d frame %2d track %3d first %3d\n", i, blockPool[i].pid,
               blockPool[i].page, blockPool[i].frame, blockPool[i].track, blockPool[i].first);
    }
    debug3("-------------------------------------------------\n\n");
}

static void tableInfo(int pid) {
    debug3("-----------------%d table info--------------------\n", pid);
    USLOSS_PTE *table = NULL;
    int rc = P3PageTableGet(pid, &table);
    assert(rc == P1_SUCCESS);
    debug3("table %p\n", table);
    for (int i = 0; i < PAGES; ++i) {
        debug3("PID %d PTE %d incore %d read %d write %d frame %d\n", pid, i, (table + i)->incore,
               (table + i)->read, (table + i)->write, (table + i)->frame);
    }
    debug3("-------------------------------------------------\n\n");
}

/*
 *----------------------------------------------------------------------
 *
 * P3SwapInit --
 *
 *  Initializes the swap data structures.
 *
 * Results:
 *   P3_ALREADY_INITIALIZED:    this function has already been called
 *   P1_SUCCESS:                success
 *
 *----------------------------------------------------------------------
 */
int P3SwapInit(int pages, int frames) {
    debug3("enter P3SwapInit with pages %d frames %d\n", pages, frames);
    if (isP3SwapInit == TRUE) {
        return P3_ALREADY_INITIALIZED;
    }
    // initialize the swap data structures, e.g. the pool of free blocks
    PAGES = pages;
    FRAMES = frames;
    // initialize frame info array
    framesInfoArr = malloc(frames * sizeof(Frame));
    if (framesInfoArr == NULL) {
        USLOSS_Console("The under layer system has no more memory. \n");
        USLOSS_Halt(1);
    }
    for (int j = 0; j < frames; ++j) {
        framesInfoArr[j].isBusy = 0;
        framesInfoArr[j].blockNum = -1;
    }

    // get info of swap disk
    int rc;
    rc = P2_DiskSize(P3_SWAP_DISK, &sector, &track, &disk);
    assert(rc == P1_SUCCESS);
    debug3("sector %d track %d disk %d\n", sector, track, disk);
    // get frame size;
    sizeOfFrame = USLOSS_MmuPageSize();
    // calculate how many sector we need for a frame
    numberOfSectorsForAFrame = sizeOfFrame / sector;
    if (sizeOfFrame % sector != 0) {
        numberOfSectorsForAFrame += 1;
    }
    // calculate how many blocks we can hold in the disk.
    numberOfBlocks = (track * disk) / numberOfSectorsForAFrame;

    // initialize the pool
    blockPool = malloc(sizeof(Block) * numberOfBlocks);
    numberOfFreeBlocks = numberOfBlocks;
    if (blockPool == NULL) {
        USLOSS_Console("The under layer system has no more memory. \n");
        USLOSS_Halt(1);
    }
    for (int i = 0; i < numberOfBlocks; ++i) {
        blockPool[i].pid = -1;
        blockPool[i].page = -1;
        blockPool[i].frame = -1;
        blockPool[i].track = -1;
        blockPool[i].first = -1;
    }

    // crate a mutex semaphore for the block pool
    semCreate("PHASE3D_BLOCK_POOL_MUTEX", 1, &blockPoolMutex, rc)
    semCreate("PHASE3D_VM_STATE_MUTEX", 1, &vmStateMutex, rc)

    P(vmStateMutex, rc)
    P3_vmStats.freeBlocks = numberOfFreeBlocks;
    P3_vmStats.blocks = numberOfBlocks;
    V(vmStateMutex, rc)

    //update the value
    isP3SwapInit = TRUE;
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3SwapShutdown --
 *
 *  Cleans up the swap data structures.
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3SwapInit has not been called
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */
int P3SwapShutdown(void) {
    debug3("enter P3SwapShutdown \n");
    if (isP3SwapInit == FALSE) {
        return P3_NOT_INITIALIZED;
    }
    int rc;
    free(blockPool);
    free(framesInfoArr);
    semFree(blockPoolMutex, rc)
    semFree(vmStateMutex, rc)
    // clean things up
    isP3SwapInit = FALSE;
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3SwapFreeAll --
 *
 *  Frees all swap space used by a process
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3SwapInit has not been called
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */

int P3SwapFreeAll(int pid) {
    if (isP3SwapInit == FALSE) {
        return P3_NOT_INITIALIZED;
    }
    /*****************
    P(mutex)
    free all swap space used by the process
    V(mutex)
    *****************/
    int rc;
    P(blockPoolMutex, rc)
    debug3("pid:%d enter P3SwapFreeAll \n", pid);
    USLOSS_PTE *table;
    rc = P3PageTableGet(pid, &table);
    assert(rc == P1_SUCCESS);
    for (int j = 0; j < PAGES; ++j) {
        if ((table + j)->incore == (unsigned int) 1) {
            framesInfoArr[(table + j)->frame].isBusy = 0;
            framesInfoArr[(table + j)->frame].blockNum = -1;
            (table + j)->incore = 0;
            (table + j)->read = 0;
            (table + j)->write = 0;
        }
    }

    for (int i = 0; i < numberOfBlocks; ++i) {
        if (blockPool[i].pid == pid) {
            blockPool[i].pid = -1;
            blockPool[i].page = -1;
            blockPool[i].frame = -1;
            blockPool[i].track = -1;
            blockPool[i].first = -1;
            numberOfFreeBlocks += 1;
        }
    }
    P(vmStateMutex, rc)
    P3_vmStats.freeBlocks = numberOfFreeBlocks;
    V(vmStateMutex, rc)
    V(blockPoolMutex, rc)
    return P1_SUCCESS;
}

/*
 *----------------------------------------------------------------------
 *
 * P3SwapOut --
 *
 * Uses the clock algorithm to select a frame to replace, writing the page that is in the frame out 
 * to swap if it is dirty. The page table of the page’s process is modified so that the page no 
 * longer maps to the frame. The frame that was selected is returned in *frame. 
 *
 * Results:
 *   P3_NOT_INITIALIZED:    P3SwapInit has not been called
 *   P1_SUCCESS:            success
 *
 *----------------------------------------------------------------------
 */
int P3SwapOut(int *frame) {
    int result = P1_SUCCESS;
    if (isP3SwapInit == FALSE) {
        return P3_NOT_INITIALIZED;
    }

    /*****************
    NOTE: in the pseudo-code below I used the notation frames[x] to indicate frame x. You 
    may or may not have an actual array with this name. As with all my pseudo-code feel free
    to ignore it.
    static int hand = -1;    // start with frame 0
    P(mutex)
    loop
        hand = (hand + 1) % # of frames
        if frames[hand] is not busy
            if frames[hand] hasn't been referenced (USLOSS_MmuGetAccess)
                target = hand
                break
            else
                clear reference bit (USLOSS_MmuSetAccess)
    if frame[target] is dirty (USLOSS_MmuGetAccess)
        write page to its location on the swap disk (P3FrameMap,P2_DiskWrite,P3FrameUnmap)
        clear dirty bit (USLOSS_MmuSetAccess)
    update page table of process to indicate page is no longer in a frame
    mark frames[target] as busy
    V(mutex)
    *frame = target
    *****************/
    int rc;
    int target;
    int accessPtr;
    P(blockPoolMutex, rc)
    debug3("enter P3SwapOut \n");
    // find a frame to swap
    while (TRUE) {
        hand = (hand + 1) % FRAMES;
        if (framesInfoArr[hand].isBusy == 0) {
            rc = USLOSS_MmuGetAccess(hand, &accessPtr);
            assert(rc == USLOSS_MMU_OK);
            if ((accessPtr & USLOSS_MMU_REF) == 0) {
                target = hand;
                break;
            } else {
                rc = USLOSS_MmuSetAccess(hand, accessPtr & ~USLOSS_MMU_REF);
                assert(rc == USLOSS_MMU_OK);
            }
        }
    }
    rc = USLOSS_MmuGetAccess(target, &accessPtr);
    assert(rc == USLOSS_MMU_OK);
    int block = framesInfoArr[target].blockNum;
    // get the page table and update it. "to indicate page is no longer in a frame"
    if (block != -1 && blockPool[block].pid != -1) {
        USLOSS_PTE *table;
        rc = P3PageTableGet(blockPool[block].pid, &table);
        assert(rc == P1_SUCCESS);
        (table + blockPool[block].page)->incore = 0;
        debug3("swap out pid %d page %d frame %d\n", blockPool[block].pid, blockPool[block].page,
               target);
    }
    //write the frame to the corresponding block.
    if (((accessPtr & USLOSS_MMU_DIRTY) >> 1) == 1 && block != -1) {
        char buffer[numberOfSectorsForAFrame * sector];
        char *addr;
        rc = P3FrameMap(target, (void **) &addr);
        assert(rc == P1_SUCCESS);
        for (int i = 0; i < sizeOfFrame; ++i) {
            buffer[i] = addr[i];
        }
        rc = P3FrameUnmap(target);
        assert(rc == P1_SUCCESS);
        rc = P2_DiskWrite(P3_SWAP_DISK, blockPool[block].track, blockPool[block].first,
                          numberOfSectorsForAFrame, buffer);
        assert(rc == P1_SUCCESS);
        rc = USLOSS_MmuSetAccess(target, accessPtr & ~USLOSS_MMU_DIRTY);
        assert(rc == USLOSS_MMU_OK);
        P(vmStateMutex, rc)
        P3_vmStats.pageOuts += 1;
        V(vmStateMutex, rc)
    }
    // update the frame
    framesInfoArr[target].isBusy = 1;
    // the frame is no longer bounded with this block.
    // because a new process is going to use it.
    framesInfoArr[target].blockNum = -1;
    V(blockPoolMutex, rc)
    *frame = target;
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * P3SwapIn --
 *
 *  Opposite of P3FrameMap. The frame is unmapped.
 *
 * Results:
 *   P3_NOT_INITIALIZED:     P3SwapInit has not been called
 *   P1_INVALID_PID:         pid is invalid      
 *   P1_INVALID_PAGE:        page is invalid         
 *   P1_INVALID_FRAME:       frame is invalid
 *   P3_EMPTY_PAGE:          page is not in swap
 *   P1_OUT_OF_SWAP:         there is no more swap space
 *   P1_SUCCESS:             success
 *
 *----------------------------------------------------------------------
 */
int P3SwapIn(int pid, int page, int frame) {
    int result = P1_SUCCESS;
    if (isP3SwapInit == FALSE) {
        return P3_NOT_INITIALIZED;
    }
    if (pid < 0 || pid >= P1_MAXPROC) {
        return P1_INVALID_PID;
    }
    if (page < 0 || page >= PAGES) {
        return P3_INVALID_PAGE;
    }
    if (frame < 0 || frame >= FRAMES) {
        return P3_INVALID_FRAME;
    }

    /*****************
    P(mutex)
    if page is on swap disk
        read page from swap disk into frame (P3FrameMap,P2_DiskRead,P3FrameUnmap)
    else
        allocate space for the page on the swap disk
        if no more space
            result = P3_OUT_OF_SWAP
        else
            result = P3_EMPTY_PAGE
    mark frame as not busy
    V(mutex)
    *****************/
    int rc;
    P(blockPoolMutex, rc)
    USLOSS_PTE *table;
    rc = P3PageTableGet(pid, &table);
    assert(rc == P1_SUCCESS);
    debug3("enter P3SwapIn with pid %d page %d frame %d\n", pid, page, frame);
    // try to find the corresponding block and a free block.
    int block = -1, freeBlock = -1;
    for (int i = 0; i < numberOfBlocks && (block == -1 || freeBlock == -1); ++i) {
        if (freeBlock == -1 && blockPool[i].pid == -1) {
            freeBlock = i;
        }
        if (block == -1 && pid == blockPool[i].pid && page == blockPool[i].page) {
            block = i;
        }
    }
    // if we find the block in the disk, we just swap in it.
    if (block != -1) {
        char buffer[numberOfSectorsForAFrame * sector];
        rc = P2_DiskRead(P3_SWAP_DISK, blockPool[block].track,
                         blockPool[block].first, numberOfSectorsForAFrame, buffer);
        P(vmStateMutex, rc)
        P3_vmStats.pageIns += 1;
        V(vmStateMutex, rc)
        assert(rc == P1_SUCCESS);

        char *addr;
        rc = P3FrameMap(frame, (void **) &addr);
        assert(rc == P1_SUCCESS);
        for (int i = 0; i < sizeOfFrame; ++i) {
            addr[i] = buffer[i];
        }
        rc = P3FrameUnmap(frame);
        assert(rc == P1_SUCCESS);
        debug3("P1_SUCCESS\n");
    } else if (numberOfFreeBlocks == 0) {
        debug3("P3_OUT_OF_SWAP\n");
        // if there is no free blocks, return out of swap
        result = P3_OUT_OF_SWAP;
    } else {
        debug3("P3_EMPTY_PAGE\n");
        // if the corresponding block can not be found, and there is a free block,
        // use that block.
        block = freeBlock;
        numberOfFreeBlocks -= 1;
        P(vmStateMutex, rc)
        P3_vmStats.freeBlocks = numberOfFreeBlocks;
        V(vmStateMutex, rc)
        blockPool[block].pid = pid;
        blockPool[block].page = page;
        blockPool[block].frame = frame;
        int sectorsBefore = block * numberOfSectorsForAFrame;
        blockPool[block].first = (sectorsBefore) % track;
        blockPool[block].track = (sectorsBefore) / track;
        result = P3_EMPTY_PAGE;
    }
    // mark frame as not busy
    framesInfoArr[frame].isBusy = 0;
    if (result != P3_OUT_OF_SWAP) {
        (table + page)->incore = 1;
        (table + page)->read = 1;
        (table + page)->write = 1;
        (table + page)->frame = (unsigned int) frame;
        framesInfoArr[frame].blockNum = block;
    }
    V(blockPoolMutex, rc)
    return result;
}