#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <usloss.h>
#include <phase1.h>

#include "phase2Int.h"

#define TAG_KERNEL 0
#define TAG_USER 1
#define P2C_ABORT_OPERATION -1

#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }

static int DiskDriver(void *);

static void DiskReadStub(USLOSS_Sysargs *sysargs);

static void DiskWriteStub(USLOSS_Sysargs *sysargs);

static void DiskSizeStub(USLOSS_Sysargs *sysargs);

static int disableAssert = FALSE;

typedef struct diskRequestInfo {
    void *arg1, *arg2, *arg3, *arg4;
    int sem, operation, returnVal;
} diskRequestInfo;

typedef struct diskRequestQueueNode {
    diskRequestInfo *info;
    struct diskRequestQueueNode *next;
} diskRequestQueueNode;

static int diskRequestQueueMutex[2];
static int IsDiskRequestQueueReady[2];
static diskRequestQueueNode *diskRequestQueueHead[2] = {NULL, NULL};
static diskRequestQueueNode *diskRequestQueueTail[2] = {NULL, NULL};
static int diskSize[2] = {0,0};
/**
 * insert into end by using tail
 * the first time is all node
 * returns whether this insert works or not
 * */
int diskRequestQueueInsert(diskRequestInfo *info, int unit) {
    mode_check();
    if (info == NULL) {
        return FALSE;
    }
    int rc, result = TRUE;
    rc = P1_P(diskRequestQueueMutex[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    diskRequestQueueNode **head = &(diskRequestQueueHead[unit]);
    diskRequestQueueNode **tail = &(diskRequestQueueTail[unit]);
    diskRequestQueueNode *node = malloc(sizeof(diskRequestQueueNode));
    node->next = NULL;
    node->info = info;
    if ((*head == NULL && *tail != NULL) || (*head != NULL && *tail == NULL)) {
        result = FALSE;
    } else if (*head == NULL && *tail == NULL) {
        *head = node;
        *tail = node;
    } else {
        (*tail)->next = node;
        *tail = node;
    }
    rc = P1_V(diskRequestQueueMutex[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_V(IsDiskRequestQueueReady[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    return result;
}
/**
 * remove top node from queue
 * and give the info to **info for using
 * */
int diskRequestQueueRemove(diskRequestInfo **info, int unit) {
    mode_check();
    if (info == NULL) {
        return FALSE;
    }
    int rc, result = TRUE;
    rc = P1_P(IsDiskRequestQueueReady[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_P(diskRequestQueueMutex[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    diskRequestQueueNode **head = &(diskRequestQueueHead[unit]);
    diskRequestQueueNode **tail = &(diskRequestQueueTail[unit]);
    if ((*head == NULL && *tail == NULL) || (*head == NULL && *tail != NULL) ||
        (*head != NULL && *tail == NULL)) {
        result = FALSE;
    } else if (*head == *tail) { // just one node in
        diskRequestQueueNode *copy = *head;
        *head = NULL;
        *tail = NULL;
        *info = copy->info;
        free(copy);
    } else { // more than one
        diskRequestQueueNode *copy = *head;
        *head = (*head)->next;
        *info = copy->info;
        free(copy);
    }
    rc = P1_V(diskRequestQueueMutex[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    return result;
}
/**
 * free all node in the queue
 * and make sure head and tail need to be NULL
 * */
void freeAllRequestQueue(int unit) {
    mode_check();
    int rc;
    rc = P1_P(diskRequestQueueMutex[unit]);
    diskRequestQueueNode **head = &(diskRequestQueueHead[unit]);
    diskRequestQueueNode **tail = &(diskRequestQueueTail[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
    if (*head != NULL && *tail != NULL) {
        diskRequestQueueNode *walker = *head;
        diskRequestQueueNode *copy;
        while (walker != NULL) {
            copy = walker;
            walker = walker->next;
            free(copy);
        }
        *head = NULL;
        *tail = NULL;
    }
    rc = P1_V(diskRequestQueueMutex[unit]);
    assert(disableAssert||rc == P1_SUCCESS);
}

/*
 * P2DiskInit
 *
 * Initialize the disk data structures and fork the disk drivers.
 */
void P2DiskInit(void) {
    mode_check();
    int rc;

    // initialize data structures here
    rc = P1_SemCreate("P2C_DISK_REQUEST_QUEUE_1_MUTEX", 1,
                      &diskRequestQueueMutex[0]);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemCreate("P2C_DISK_REQUEST_QUEUE_2_MUTEX", 1,
                      &diskRequestQueueMutex[1]);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemCreate("P2C_DISK_REQUEST_QUEUE_1_READY", 0,
                      &IsDiskRequestQueueReady[0]);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemCreate("P2C_DISK_REQUEST_QUEUE_2_READY", 0,
                      &IsDiskRequestQueueReady[1]);
    assert(disableAssert||rc == P1_SUCCESS);
    // install system call stubs here

    rc = P2_SetSyscallHandler(SYS_DISKREAD, DiskReadStub);
    assert(disableAssert||rc == P1_SUCCESS);

    // self added
    rc = P2_SetSyscallHandler(SYS_DISKWRITE, DiskWriteStub);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P2_SetSyscallHandler(SYS_DISKSIZE, DiskSizeStub);
    assert(disableAssert||rc == P1_SUCCESS);

    // fork the disk drivers here
    int pid1;
    int pid2;
    rc = P1_Fork("phase2c_disk_driver_0", DiskDriver, 0, USLOSS_MIN_STACK, 2,
                 TAG_KERNEL, &pid1);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_Fork("phase2c_disk_driver_1", DiskDriver, (void *) 1,
                 USLOSS_MIN_STACK, 2, TAG_KERNEL, &pid2);
    assert(disableAssert||rc == P1_SUCCESS);

    int sector0;
    int sector1;
    int track0;
    int track1;
    rc = P2_DiskSize(0, &sector0, &track0, &(diskSize[0]));
    if (rc == P1_INVALID_UNIT) {
        diskSize[0] = -1;
    }
    rc = P2_DiskSize(1, &sector1, &track1, &(diskSize[1]));
    if (rc == P1_INVALID_UNIT) {
        diskSize[1] = -1;
    }

}

/*
 * P2DiskShutdown
 *
 * Clean up the disk data structures and stop the disk drivers.
 */

void P2DiskShutdown(void) {
    mode_check();
    int rc, pid, status;
    diskRequestInfo info1 = {.operation = P2C_ABORT_OPERATION};
    diskRequestInfo info2 = {.operation = P2C_ABORT_OPERATION};
    rc = P1_SemCreate("P2_DISKShutdown_SEMAPHORE_1", 0, &(info1.sem));
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemCreate("P2_DISKShutdown_SEMAPHORE_2", 0, &(info2.sem));
    assert(disableAssert||rc == P1_SUCCESS);
    // free all
    freeAllRequestQueue(0);
    freeAllRequestQueue(1);
    // insert abort to queue
    diskRequestQueueInsert(&info1, 0);
    diskRequestQueueInsert(&info2, 1);
    rc = P1_P(info1.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_P(info2.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemFree(info1.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemFree(info2.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    assert(disableAssert||diskRequestQueueHead[0] == NULL);
    assert(disableAssert||diskRequestQueueHead[1] == NULL);
    assert(disableAssert||diskRequestQueueTail[0] == NULL);
    assert(disableAssert||diskRequestQueueTail[1] == NULL);
    rc = P1_Join(0, &pid, &status);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_Join(0, &pid, &status);
    assert(disableAssert||rc == P1_SUCCESS);
}

// self made
// helper function for wait
static int DiskDriverWait(int unit) {
    mode_check();
    int rc, status;
    while (TRUE) {
        rc = P1_WaitDevice(USLOSS_DISK_DEV, unit, &status);
        if (rc == P1_WAIT_ABORTED || status == USLOSS_DEV_READY) {
            break;
        }
        if (status == USLOSS_DEV_BUSY) { // it should never return busy
            rc = P2_Sleep(1);
            assert(disableAssert||rc == P1_SUCCESS);
            continue;
        }
        return USLOSS_DEV_ERROR;
    }
    return P1_SUCCESS;
}

// This function performs a seek operation.
static int DiskDriverSeekHelper(int unit, int track) {
    mode_check();
    int rc, result;
    USLOSS_DeviceRequest request;
    request.opr = USLOSS_DISK_SEEK;
    request.reg1 = (void *) track;
    request.reg2 = (void *) -1;
    rc = USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &request);
    assert(disableAssert||rc == USLOSS_DEV_OK);
    result = DiskDriverWait(unit);
    return result;
}


// self made
static int DiskDriverReadWriteHelper(int unit, int track, int first,
                                     int sectors, int operation, void *buffer) {
    mode_check();
    USLOSS_DeviceRequest request;
    int currentTrack = track, currentSector = first;
    int i, rc, result;
    char *bufferWalker = (char *) buffer;
    // while request isn't complete
    result = DiskDriverSeekHelper(unit, currentTrack);
    if (result != P1_SUCCESS) {
        return result;
    }
    for (i = 0; i < sectors;) {
        if (currentSector >= USLOSS_DISK_TRACK_SIZE) { // move track
            ++currentTrack;
            currentSector = 0;
            result = DiskDriverSeekHelper(unit, currentTrack);
            if (result != P1_SUCCESS) {
                return result;
            }
        } else {// read or write
            request.opr = operation;
            request.reg1 = (void *) currentSector++;
            request.reg2 = bufferWalker;
            rc = USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &request);
            assert(disableAssert||rc == USLOSS_DEV_OK);
            result = DiskDriverWait(unit);
            bufferWalker += USLOSS_DISK_SECTOR_SIZE;
            ++i;
        }
        if (result != P1_SUCCESS) {
            return result;
        }
    }
    return result;
}

/*
 * DiskDriver
 *
 * Kernel process that manages a disk device and services disk I/O requests from other processes.
 * Note that it may require several disk operations to service a single I/O request.
 */
static int
DiskDriver(void *arg) {
    mode_check();
    int unit = (int) arg, rc;
    diskRequestInfo *info;
    // repeat
    while (TRUE) {
        // wait for next request
        // (remove from the queue, our remove waits)
        rc = diskRequestQueueRemove(&info, unit);
        if (rc != TRUE) {
            USLOSS_Console("QUEUE ERROR\n");
            break;
        }
        if (info->operation == USLOSS_DISK_READ ||
            info->operation == USLOSS_DISK_WRITE) {
            rc = DiskDriverReadWriteHelper(unit, (int) info->arg1,
                                           (int) info->arg2, (int) info->arg3,
                                           info->operation,
                                           info->arg4);
            info->returnVal = rc;
        } else if (info->operation == USLOSS_DISK_TRACKS) {
            USLOSS_DeviceRequest request = {.opr=USLOSS_DISK_TRACKS, .reg1 =
            info->arg1, .reg2=(void *) -1};
            rc = USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &request);
            // deal with return value
            if (rc == USLOSS_DEV_OK) {
                rc = DiskDriverWait(unit);
            }
            info->returnVal = rc;
        } else {
            rc = USLOSS_DEV_ERROR;
        }
        // update the request status and wake the waiting process
        info->returnVal = rc;
        rc = P1_V(info->sem);
        assert(disableAssert||rc == P1_SUCCESS);
        // until P2DiskShutdown has been called
        if (info->operation == P2C_ABORT_OPERATION) {
            break;
        }
    }
    return P1_SUCCESS;
}

// every track has 16 sectors
// each sector is 512 bytes
// first is which sectors
int P2DiskReaderWriterHelper(int unit, int track, int first, int sectors,
                             void *buffer, int operation) {
    mode_check();
    if (sectors == 0) {
        return P1_SUCCESS;
    }
    if ((unit != 0 && unit != 1) || diskSize[unit] == -1) {
        return P1_INVALID_UNIT;
    }
    if (buffer == NULL) {
        return P2_NULL_ADDRESS;
    }
    if (first >= USLOSS_DISK_TRACK_SIZE || first < 0) {
        return P2_INVALID_FIRST;
    }
    if (track < 0 || track >= diskSize[unit]) {
        return P2_INVALID_TRACK;
    }
    // total_size - current_size + rest sectors
    if (sectors > USLOSS_DISK_TRACK_SIZE * (diskSize[unit] - track - 1) +
                  (USLOSS_DISK_TRACK_SIZE - first) || sectors < 0)
        return P2_INVALID_SECTORS;
    // give request to the proper device driver
    // (make the struct)
    diskRequestInfo info = {.arg1 = (void *) track, .arg2 = (void *) first, .arg3 = (void *) sectors, .arg4 = buffer, .sem = -1, .operation = operation};
    char name[20];
    int rc;
    sprintf(name, "P2_DISK_SEMAPHORE_%d", P1_GetPid());
    rc = P1_SemCreate(name, 0, &(info.sem));
    assert(disableAssert||rc == P1_SUCCESS);
    // (insert to the queue)
    diskRequestQueueInsert(&info, unit);
    // wait until device driver completes the request (P on semaphore)
    rc = P1_P(info.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemFree(info.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    return P1_SUCCESS;
}

/*
 * P2_DiskRead
 *
 * Reads the specified number of sectors from the disk starting at the specified track and sector.
 */
int P2_DiskRead(int unit, int track, int first, int sectors, void *buffer) {
    mode_check();
    return P2DiskReaderWriterHelper(unit, track, first, sectors, buffer,
                                    USLOSS_DISK_READ);
}

/*
 * self made P2_DiskWrite
 *
 * Writes the specified number of sectors from the disk starting at the specified track and sector.
 */
int P2_DiskWrite(int unit, int track, int first, int sectors, void *buffer) {
    mode_check();
    return P2DiskReaderWriterHelper(unit, track, first, sectors, buffer,
                                    USLOSS_DISK_WRITE);
}

/*
 * self made P2_DiskSize
 *
 * Returns information about the size of the disk indicated by unit.
 * The sector parameter is filled in with the number of bytes in a sector,
 * track with the number of sectors in a track, and disk with the number of tracks in the disk.
 */
int P2_DiskSize(int unit, int *sector, int *track, int *disk) {
    // how to do this?
    mode_check();

    if ((unit != 0 && unit != 1) || diskSize[unit] == -1) {
        return P1_INVALID_UNIT;
    }
    if (sector == NULL || track == NULL || disk == NULL) {
        return P2_NULL_ADDRESS;
    }
    *sector = USLOSS_DISK_SECTOR_SIZE;
    *track = USLOSS_DISK_TRACK_SIZE;
    diskRequestInfo info = {.arg1 = (void *) disk, .sem = -1, .operation = USLOSS_DISK_TRACKS};
    char name[20];
    int rc;
    sprintf(name, "P2_DISK_SEMAPHORE_%d", P1_GetPid());
    rc = P1_SemCreate(name, 0, &(info.sem));
    assert(disableAssert||rc == P1_SUCCESS);
    // (insert to the queue)
    diskRequestQueueInsert(&info, unit);
    // wait until device driver completes the request (P on semaphore)
    rc = P1_P(info.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    rc = P1_SemFree(info.sem);
    assert(disableAssert||rc == P1_SUCCESS);
    if (info.returnVal != USLOSS_DEV_OK) {
        return P1_INVALID_UNIT;
    }
    return P1_SUCCESS;
}

/*
 * DiskReadStub
 *
 * Stub for the Sys_DiskRead system call.
 */
static void
DiskReadStub(USLOSS_Sysargs *sysargs) {
    mode_check();
    // unpack sysargs
    // call P2_DiskRead
    int rc = P2_DiskRead((int) sysargs->arg5, (int) sysargs->arg3,
                         (int) sysargs->arg4, (int) sysargs->arg2,
                         sysargs->arg1);
    // put result in sysargs
    sysargs->arg4 = (void *) rc;
}

/*
 * self made DiskWriteStub
 *
 * Stub for the Sys_DiskWrite system call.
 */
static void
DiskWriteStub(USLOSS_Sysargs *sysargs) {
    mode_check();
    // unpack sysargs
    // call P2_DiskWrite
    int rc = P2_DiskWrite((int) sysargs->arg5, (int) sysargs->arg3,
                          (int) sysargs->arg4, (int) sysargs->arg2,
                          sysargs->arg1);
    // put result in sysargs
    sysargs->arg4 = (void *) rc;
}

/*
 * self made DiskSizeStub
 *
 * Stub for the Sys_DiskSize system call.
 */
static void
DiskSizeStub(USLOSS_Sysargs *sysargs) {
    mode_check();
    // unpack sysargs
    // call P2_DiskSize
    int unit = (int) sysargs->arg1;
    int sector, track, disk;
    int rc = P2_DiskSize(unit, &sector, &track, &disk);
    sysargs->arg1 = (void *) sector;
    sysargs->arg2 = (void *) track;
    sysargs->arg3 = (void *) disk;
    // put result in sysargs
    sysargs->arg4 = (void *) rc;
}