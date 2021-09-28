#include "phase1.h"
#include <stdio.h>
#include <assert.h>



int child(void *arg) {
    int rc;
    USLOSS_Console("child");
    rc = P1_WakeupDevice(USLOSS_CLOCK_DEV, 0,100 , 0);
    assert(rc == P1_SUCCESS);
    rc = P1_WakeupDevice(USLOSS_ALARM_DEV, 0,101 , 0);
    assert(rc == P1_SUCCESS);
    rc = P1_WakeupDevice(USLOSS_DISK_DEV, 0,102 , 0);
    assert(rc == P1_SUCCESS);
    rc = P1_WakeupDevice(USLOSS_TERM_DEV, 0,103 , 0);
    assert(rc == P1_SUCCESS);
    return 0;
}

int P2_Startup(void *arg){

    int rc;
    int pid;
    USLOSS_Console(" \n---------Starting Test MUL ----------\n");
    rc = P1_Fork("child", child, NULL, USLOSS_MIN_STACK, 4 , 0, &pid);
    assert(rc == P1_SUCCESS);
    //rc = P1_SemCreate("semaphore" , 0, &sem1 );
    int now;
    USLOSS_Console("Clock is going to wait\n");
    rc = P1_WaitDevice(USLOSS_CLOCK_DEV, 0, &now);
    assert(rc == P1_SUCCESS);
    assert(now == 100);
    USLOSS_Console("Clock is weaken right now\n");

    USLOSS_Console("Alarm is going to wait\n");
    rc = P1_WaitDevice(USLOSS_ALARM_DEV, 0, &now);
    assert(rc == P1_SUCCESS);
    assert(now == 101);
    USLOSS_Console("ALARM is weaken right now\n");

    USLOSS_Console("Disk is going to wait\n");
    rc = P1_WaitDevice(USLOSS_DISK_DEV, 0, &now);
    assert(rc == P1_SUCCESS);
    assert(now == 102);
    USLOSS_Console("Disk is weaken right now\n");

    USLOSS_Console("Term is going to wait\n");
    rc = P1_WaitDevice(USLOSS_TERM_DEV, 0, &now);
    assert(rc == P1_SUCCESS);
    assert(now == 103);
    USLOSS_Console("Term is weaken right now\n");
    USLOSS_Console(" ---------Ending Test MUL ----------\n");
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
