/*
Phase 1b 
*/

#include "phase1Int.h"
#include "usloss.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define enable(previous) \
    if (previous == 1)   \
        P1EnableInterrupts();
#define mode_check()                                      \
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) != 1) \
    {                                                     \
        USLOSS_IllegalInstruction();                      \
    }

typedef struct PCB {
    int cid;                   // context's ID
    int cpuTime;               // process's running time
    char name[P1_MAXNAME + 1]; // process's name
    int priority;              // process's priority
    P1_State state;            // state of the PCB
    // more fields here
    int isBeingUsed;              // self make flag
    int parent;                   // self add parent PID
    int children[P1_MAXPROC];     // self add childen PIDs
    int numChildren;              // self add # of children
    int sid;                      // self add sid
    int tag;                      // self add tag
    int (*func)(void *);          // self add function
    int status;                   // self made for quit and getChildrenStatus
    int quitChildren[P1_MAXPROC]; // self made for checking quit children
    int numQuitChildren;          // self made size for quitChildren
} PCB;

typedef struct dispatchNode {
    int pid;
    int priority;
} dispatchNode;

static PCB processTable[P1_MAXPROC]; // the process table
static int isTheFirstProcess = TRUE;

// self made
static int queueSize = 0;
static dispatchNode dispatchQueue[P1_MAXPROC];

// insert and delete for queue in dispatch
int dispatchQueueInsert(int pid, int priority) {
    if (queueSize == P1_MAXPROC) {
        return 0;
    }
    int i;
    for (i = 0; i < queueSize; i++) {
        if(dispatchQueue[i].pid ==pid){
            return 1;
        }
    }
    int index = queueSize++;
    dispatchQueue[index].pid = pid;
    dispatchQueue[index].priority = priority;
    return 1;
}

int dispatchQueueRemove() {
    dispatchNode node = dispatchQueue[0];
    int i;
    for (i = 1; i < queueSize; i++) {
        dispatchQueue[i - 1] = dispatchQueue[i];
    }
    queueSize -= 1;
    return node.pid;
}

void dispatchQueueSwap(int i, int j) {
    if (i==j){
        return;
    }
    dispatchNode temp = dispatchQueue[i];
    dispatchQueue[i] = dispatchQueue[j];
    dispatchQueue[j] = temp;
}

void dispatchQueueShift(int i) {
    dispatchNode node = dispatchQueue[i];
    int j;
    for (j = i + 1; j < queueSize; j++) {
        dispatchQueue[j - 1] = dispatchQueue[j];
    }
    dispatchQueue[queueSize - 1] = node;
}
int dispatchQueueRemoveByPID(int pid){
      int i=0;
      for(;i<queueSize;i++){
          // we need to remove it
          if(pid == dispatchQueue[i].pid){
             dispatchQueueShift(i);
             queueSize-=1;
             return pid;
          }
      }
      return -1;
}

// done
void P1ProcInit(void) {
    mode_check();
    P1ContextInit();
    // initialize everything including the processTable
    int i;
    for (i = 0; i < P1_MAXPROC; i++) {
        processTable[i].isBeingUsed = 0;
    }
}

// done
// todo may be use the 1st one in queue
int P1_GetPid(void) {
    mode_check();
    int previous = P1DisableInterrupts();
    int i, result = -1;

    for (i = 0; i < P1_MAXPROC; i++) {
        if (processTable[i].isBeingUsed &&
            processTable[i].state == P1_STATE_RUNNING) {
            result = i;
            break;
        }
    }
    enable(previous);
    return result;
}

// self make
static void wrapper(void *arg) {
    //to do need to figure out when turnof interrupt
    enable(1);
    // assert(processTable[ dispatchQueue[0].pid].startFunc != NULL); //???????
    P1_Quit(processTable[dispatchQueue[0].pid].func(
            arg)); // todo figure out status
}

int
P1_Fork(char *name, int (*func)(void *), void *arg, int stacksize, int priority,
        int tag, int *pid) {

    int result = P1_SUCCESS;

    // check for kernel mode
    // disable interrupts
    mode_check();
    int previous = P1DisableInterrupts();
    // check all parameters
    if (tag != 0 && tag != 1) {
        result = P1_INVALID_TAG;
    } else if (priority < 1 || priority > 6 ||
               (!isTheFirstProcess && priority == 6)) {
        result = P1_INVALID_PRIORITY;
    } else if (stacksize < USLOSS_MIN_STACK) {
        result = P1_INVALID_STACK;
    } else if (name == NULL) {
        result = P1_NAME_IS_NULL;
    } else if (strlen(name) > P1_MAXNAME) {
        result = P1_NAME_TOO_LONG;
    } else {
        int i;
        int j = -1;
        for (i = 0; i < P1_MAXPROC; i++) {
            if (processTable[i].isBeingUsed == 0) {
                if (j == -1) { // make sure that we are using the 1st empty PCB
                    j = i;
                }
                continue;
            }
            if (strcmp(processTable[i].name, name) == 0) {
                result = P1_DUPLICATE_NAME;
                break;
            }
        }
        if (result == P1_SUCCESS && j == -1) {
            result = P1_TOO_MANY_PROCESSES;
        }
        if (result == P1_SUCCESS) {
            // first process should be set in here

            int contextCreateStatus = P1ContextCreate(wrapper, arg, stacksize,
                                                      &(processTable[j].cid));
            assert(contextCreateStatus == P1_SUCCESS);
            processTable[j].cpuTime = 0;
            strcpy(processTable[j].name, name);
            processTable[j].priority = priority;
            processTable[j].tag = tag;
            *pid = j;
            processTable[j].func = func;
            processTable[j].isBeingUsed = 1;
            processTable[j].numChildren = 0;
            // modify
            processTable[j].numQuitChildren = 0;
            processTable[j].sid = -1;
            processTable[j].state = P1_STATE_RUNNING;
            processTable[j].parent = isTheFirstProcess ? -1
                                                       : dispatchQueue[0]
                                             .pid;

            dispatchQueueInsert(j, processTable[j].priority);
            if (!isTheFirstProcess) {
                processTable[dispatchQueue[0].pid].children[(processTable[dispatchQueue[0].pid].numChildren)++] = j;
                //if this is not the 1fs process, we put this new process to its parent's children list.
            }
            if (isTheFirstProcess == TRUE ||
                priority < processTable[dispatchQueue[0].pid].priority) {
                isTheFirstProcess = FALSE;
                // dispatchQueueInsert(j, processTable[j].priority);
                P1Dispatch(FALSE);
            }
        }
    }

    // create a context using P1ContextCreate
    // allocate and initialize PCB
    // if this is the first process or this process's priority is higher than the
    //    currently running process call P1Dispatch(FALSE)
    // re-enable interrupts if they were previously enabled
    enable(previous);
    return result;
}

void P1_Quit(int status) {


    // check for kernel mode
    // disable interrupts
    mode_check();
    int previous = P1DisableInterrupts();
    // remove from ready queue, set status to P1_STATE_QUIT
    int pid = P1_GetPid();
    P1SetState(pid, P1_STATE_QUIT, 0);
    processTable[pid].status = status;
    // if first process verify it doesn't have children, otherwise give children to first process
    if (pid == 0 && (processTable[0].numChildren > 0 ||
                     processTable[0].numQuitChildren > 0)) {
        USLOSS_Console("First process quitting with children, halting.");
        enable(previous);
        USLOSS_Halt(1);
    }
    if (pid != 0) {
        // children change their parent
        int i;
        for (i = 0; i < processTable[pid].numChildren; i++) {
            processTable[processTable[pid].children[i]].parent = 0;
            processTable[0].children[processTable[0].numChildren++] = processTable[pid].children[i];
        }
    }
    // add ourself to list of our parent's children that have quit

    // not sure 
    if(pid ==0 ){
         P1Dispatch(FALSE);
         enable(previous);
         return ;
    }
    PCB *parent = &(processTable[processTable[pid].parent]);
    parent->quitChildren[parent->numQuitChildren++] = pid;
    int i, k;
    for (i = 0; i < parent->numChildren; i++) {
        if (parent->children[i] == pid) {
            for (k = i + 1; k < parent->numChildren; k++) {
                parent->children[k - 1] = parent->children[k];
            }
        }
    }
    parent->numChildren -= 1;
    // if parent is in state P1_STATE_JOINING set its state to P1_STATE_READY
    if (processTable[processTable[pid].parent].state == P1_STATE_JOINING) {
        P1SetState(processTable[pid].parent, P1_STATE_READY, 0);
    }
    P1Dispatch(FALSE);
    enable(previous);
    // should never get here
    assert(0);
}

int P1GetChildStatus(int tag, int *cpid, int *status) {
    mode_check();
    int previous = P1DisableInterrupts();
    int result = P1_SUCCESS;
    // do stuff here
    if (tag != 0 && tag != 1) {
        result = P1_INVALID_TAG;
    } else if (processTable[dispatchQueue[0].pid].numQuitChildren == 0) {
        result = P1_NO_QUIT;
    } else {
        int hasChildWithTag = FALSE;
        int target = -1;
        int i = 0;
        PCB *parent = &(processTable[dispatchQueue[0].pid]);

        for (; i < parent->numQuitChildren; i++) {
            if (processTable[parent->quitChildren[i]].tag == tag) {
                hasChildWithTag = TRUE;
                target = parent->quitChildren[i];
                break;
            }
        }
        if (!hasChildWithTag) {
            result = P1_NO_CHILDREN;
        } else {
            *cpid = target;
            *status = processTable[target].status;
            processTable[target].isBeingUsed = 0;
            // P1SetState(target, P1_STATE_FREE, 0);
            processTable[target].state = P1_STATE_FREE;
            P1ContextFree(processTable[target].cid);
            for (i += 1; i < parent->numQuitChildren; i++) {
                parent->quitChildren[i - 1] = parent->quitChildren[i];
            }
            parent->numQuitChildren -= 1;
            P1ContextFree(processTable[target].cid);
        }
    }

    enable(previous);
    return result;
}

int P1SetState(int pid, P1_State state, int sid) {
    // do stuff here
    mode_check();
    int previous = P1DisableInterrupts();
    int result = P1_SUCCESS;
    if (state != P1_STATE_READY && state != P1_STATE_JOINING &&
        state != P1_STATE_BLOCKED && state != P1_STATE_QUIT) {
        result = P1_INVALID_STATE;
    } else if (pid < 0 || pid >= P1_MAXPROC ||
               processTable[pid].isBeingUsed == 0) {
        result = P1_INVALID_PID;
    } else {
        if (state == P1_STATE_JOINING &&
            processTable[pid].numQuitChildren > 0) {
            result = P1_CHILD_QUIT;
        } else {
            // normal situation
            processTable[pid].state = state;
            if (state ==
                P1_STATE_BLOCKED) {
                processTable[pid].sid = sid;
            }
            if(state!=P1_STATE_READY){
                dispatchQueueRemoveByPID(pid);
            }else{
                dispatchQueueInsert(pid,processTable[pid].priority);
            }
        }
    }
    enable(previous);
    return result;
}

void P1Dispatch(int rotate) {
    // select the highest-priority runnable process
    // call P1ContextSwitch to switch to that process
    mode_check();
    int previous = P1DisableInterrupts();
    // int i ;
    // dispatchNode* highestPriority = NULL;

    // one process only

    if (queueSize == 1) {
        //  P1SetState(dispatchQueue[0].pid, P1_STATE_RUNNING, 0);
        processTable[dispatchQueue[0].pid].state = P1_STATE_RUNNING;
        enable(1);
        P1ContextSwitch(processTable[dispatchQueue[0].pid].cid);
        return;
    }

    if (queueSize == 0) {
        USLOSS_Console("No runnable processes, halting.");
        enable(1);
        USLOSS_Halt(0);
    }
    int highestIndex = -1;
    int i;

    for (i = 1; i < queueSize; i++) {
        if (highestIndex == -1 ||
            dispatchQueue[highestIndex].priority > dispatchQueue[i].priority) {
            highestIndex = i;
        }
    }
    if (dispatchQueue[highestIndex].priority < dispatchQueue[0]
    .priority) {
        // run highest and put cur to end of queue
        // we swap the highest and 0
        // 0 need to be ready and highest should be running
        dispatchQueueSwap(0, highestIndex);
        // P1SetState(dispatchQueue[0].pid, P1_STATE_RUNNING, 0);
        processTable[dispatchQueue[0].pid].state = P1_STATE_RUNNING;
        P1SetState(dispatchQueue[highestIndex].pid, P1_STATE_READY, 0);
    } else if (rotate == 0 || dispatchQueue[highestIndex].priority >
                              dispatchQueue[0].priority) {
        // run current set it to running
        // make assert
        // if it is not running , we need to set it to running
        // P1SetState(dispatchQueue[0].pid, P1_STATE_RUNNING, 0);
        processTable[dispatchQueue[0].pid].state = P1_STATE_RUNNING;
    } else {
        // run next one which has same
        // we swap highest and 0
        // 0 need to be ready and highest should be running
        // we put 0  to the end
        dispatchQueueSwap(0, highestIndex);
        // P1SetState(dispatchQueue[0].pid, P1_STATE_RUNNING, 0);
        processTable[dispatchQueue[0].pid].state = P1_STATE_RUNNING;
        P1SetState(dispatchQueue[highestIndex].pid, P1_STATE_READY, 0);
        dispatchQueueShift(highestIndex);
    }

    enable(1);
    int rc = P1ContextSwitch(processTable[dispatchQueue[0].pid].cid);
    assert(rc == P1_SUCCESS);
}

//done
int P1_GetProcInfo(int pid, P1_ProcInfo *info) {
    int result = P1_SUCCESS;
    // fill in info here
    mode_check();
    int previous = P1DisableInterrupts();
    if (!processTable[pid].isBeingUsed) {
        result = P1_INVALID_PID;
    } else {
        strcpy(info->name, processTable[pid].name);
        info->state = processTable[pid].state;
        info->sid = processTable[pid].sid;
        info->priority = processTable[pid].priority;
        info->tag = processTable[pid].tag;
        info->cpu = processTable[pid].cpuTime;
        info->parent = processTable[pid].parent;
        int i;
        for (i = 0; i < P1_MAXPROC; i++) {
            (info->children)[i] = processTable[pid].children[i];
        }
        // fix children number
        info->numChildren = processTable[pid].numChildren +  processTable[pid].numQuitChildren;
    }
    enable(previous);
    return result;
}