#include "phase1.h"
#include <stdio.h>
#include <assert.h>



int child(void *arg) {
    int rc;
    USLOSS_Console("child");
    rc = P1_WakeupDevice(USLOSS_CLOCK_DEV, 0,100 , 0);
    
    assert(rc == P1_SUCCESS);
    rc = P1_WakeupDevice(USLOSS_CLOCK_DEV, 0,101 , 1);
    
    assert(rc == P1_SUCCESS);
    USLOSS_Console("here we go\n");
    return 0;
}

int P2_Startup(void *arg){

    int rc;
    int pid;
    USLOSS_Console(" \n---------Starting Test Complete ----------\n");
    rc = P1_Fork("child", child, NULL, USLOSS_MIN_STACK, 4 , 0, &pid);
    assert(rc == P1_SUCCESS);
    //rc = P1_SemCreate("semaphore" , 0, &sem1 );
    int now;
    USLOSS_Console("Clock0 is going to wait\n");
    rc = P1_WaitDevice(USLOSS_CLOCK_DEV, 0, &now);
    assert(rc == P1_SUCCESS);
    assert(now == 100);
    USLOSS_Console("Clock0 is weaken right now\n");


    
    USLOSS_Console("Clock1 is going to wait\n");
    rc = P1_WaitDevice(USLOSS_CLOCK_DEV, 0, &now);
    assert(rc == P1_WAIT_ABORTED);
    assert(now == 100);
    USLOSS_Console("Clock1 is weaken right now\n");

    USLOSS_Console("Clock is not going to success\n");
    rc = P1_WaitDevice(USLOSS_CLOCK_DEV, -1, &now);
    assert(rc == P1_INVALID_UNIT);

    USLOSS_Console("Alarm is not going to success\n");
    rc = P1_WaitDevice(USLOSS_ALARM_DEV, -1, &now);
    assert(rc == P1_INVALID_UNIT);

    USLOSS_Console("DISK is not going to success\n");
    rc = P1_WaitDevice(USLOSS_DISK_DEV, -1, &now);
    assert(rc == P1_INVALID_UNIT);
    
    USLOSS_Console("TERM is not going to success\n");
    rc = P1_WaitDevice(USLOSS_TERM_DEV, -1, &now);
    assert(rc == P1_INVALID_UNIT);
    

    

   
    USLOSS_Console(" ---------Ending Test Complete ----------\n");
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
