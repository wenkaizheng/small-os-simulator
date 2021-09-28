#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "usloss.h"
#include "phase1Int.h"

#define enable(previous) \
    if (previous == 1)   \
        P1EnableInterrupts();
#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }

typedef struct queueNode {
    int pid;
    struct queueNode *next;
} queueNode;

typedef struct Sem {
    char name[P1_MAXNAME + 1];
    u_int value;
    // more fields here
    int isBeingUsed; // self make
    int queueSize; // self make
    queueNode *head;  // self make

} Sem;


static Sem sems[P1_MAXSEM];


queueNode *insert(int pid, queueNode *node) {
    queueNode *head = malloc(sizeof(queueNode));
    head->next = NULL;
    head->pid = pid;
    if (node == NULL) {
        return head;
    } else {
        queueNode *walker = node;
        while (walker->next != NULL) {
            walker = walker->next;
        }
        walker->next = head;
        return node;
    }
}

int remove(queueNode **node) {
    if (*node == NULL) {
        return -1;
    } else {
        queueNode *head = *node;
        int pid = head->pid;
        *node = (*node)->next;
        free(head);
        return pid;
    }
}


void P1SemInit(void) {
    P1ProcInit();
    mode_check();
    // initialize sems here
    int i;
    for (i = 0; i < P1_MAXSEM; i++) {
        sems[i].isBeingUsed = 0;
    }
}

int P1_SemCreate(char *name, unsigned int value, int *sid) {

    int result = P1_SUCCESS;
    // check for kernel mode
    mode_check();
    // disable interrupts
    int previous = P1DisableInterrupts();
    // check parameters
    if (name == NULL) {
        result = P1_NAME_IS_NULL;
    } else if (strlen(name) > P1_MAXNAME) {
        result = P1_NAME_TOO_LONG;
    } else {

        // find a free Sem and initialize it
        int i = 0;
        int j = -1;
        for (; i < P1_MAXSEM; i++) {
            if (j == -1 && sems[i].isBeingUsed == 0) {
                j = i;
            }
            if (sems[i].isBeingUsed == 1 && strcmp(name, sems[i].name) == 0) {
                result = P1_DUPLICATE_NAME;
            }
        }
        if (result == P1_SUCCESS && j == -1) {
            result = P1_TOO_MANY_SEMS;
        }
        if (result == P1_SUCCESS) {
            sems[j].isBeingUsed = 1;
            strcpy(sems[j].name, name);
            sems[j].value = value;
            *sid = j;
            //assign a queue and queueSize
            sems[j].head = NULL;
            sems[j].queueSize = 0;
        }
    }
    enable(previous);
    // re-enable interrupts if they were previously enabled
    return result;
}

int P1_SemFree(int sid) {
    mode_check();
    int previous = P1DisableInterrupts();
    int result = P1_SUCCESS;
    // more code here
    if (sid < 0 || sid >= P1_MAXSEM || sems[sid].isBeingUsed == 0) {
        result = P1_INVALID_SID;
    } else if (sems[sid].head != NULL) {
        result = P1_BLOCKED_PROCESSES;
    } else {
        sems[sid].isBeingUsed = 0;
    }
    enable(previous);
    return result;
}

int P1_P(int sid) {

    int result = P1_SUCCESS;
    if (sid < 0 || sid >= P1_MAXSEM || sems[sid].isBeingUsed == 0) {
        return P1_INVALID_SID;
    }
    // check for kernel mode
    mode_check();
    // todo do we need 2 disable interrupt here or inside of the loop?
    // disable interrupts
    int previous;
    // while value == 0
    while (1) {
        previous = P1DisableInterrupts();
        if (sems[sid].value > 0) {
            // value--
            sems[sid].value--;
            break;
        }
        //set state to P1_STATE_BLOCKED
        int pid = P1_GetPid();
        int rc = P1SetState(pid, P1_STATE_BLOCKED, sid);
        sems[sid].head = insert(pid, sems[sid].head);
        enable(previous);
        P1Dispatch(FALSE);
    }
    // re-enable interrupts if they were previously enabled
    enable(previous);
    return result;
}

int P1_V(int sid) {
    int result = P1_SUCCESS;
    if (sid < 0 || sid >= P1_MAXSEM || sems[sid].isBeingUsed == 0) {
        return P1_INVALID_SID;
    }
    // check for kernel mode
    mode_check();
    // disable interrupts
    int previous = P1DisableInterrupts();
    // value++
    sems[sid].value++;
    // if a process is waiting for this semaphore
    if (sems[sid].head != NULL) {
        //      set the process's state to P1_STATE_READY
        int pid = remove(&(sems[sid].head));
        P1SetState(pid, P1_STATE_READY, sid);
        //todo do we need to call dispatch here? see john's lecture note
        P1Dispatch(FALSE);
    }
    // re-enable interrupts if they were previously enabled
    enable(previous);
    return result;
}

int P1_SemName(int sid, char *name) {
    mode_check();
    // disable interrupts 
    // todo do we need to check mode and disable interrupt
    int previous = P1DisableInterrupts();
    int result = P1_SUCCESS;

    // more code here
    if (name == NULL) {
        result = P1_NAME_IS_NULL;
    } else if (sid < 0 || sid >= P1_MAXSEM || sems[sid].isBeingUsed == 0) {
        result = P1_INVALID_SID;
    } else {
        strcpy(name, sems[sid].name);
    }
    enable(previous);
    return result;
}