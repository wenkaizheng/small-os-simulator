#include "phase1.h"
#include <assert.h>
int P2_Startup(void *notused) 
{
    USLOSS_Console("P2_Startup\n");
    int status;
    assert(P1_WaitDevice(USLOSS_CLOCK_DEV, 1, &status)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 8\n");
    assert(P1_WaitDevice(USLOSS_ALARM_DEV, 1, &status)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 10\n");
    assert(P1_WaitDevice(USLOSS_DISK_DEV, 2, &status)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 12\n");
    assert(P1_WaitDevice(USLOSS_TERM_DEV, 4, &status)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 14\n");
    assert(P1_WaitDevice(-1, 1, &status)==P1_INVALID_TYPE);
    USLOSS_Console("P2_Startup 16\n");
    assert(P1_WakeupDevice(USLOSS_CLOCK_DEV, 1, USLOSS_DEV_READY, 1)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 18\n");
    assert(P1_WakeupDevice(USLOSS_ALARM_DEV, 1,USLOSS_DEV_READY, 1)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 20\n");
    assert(P1_WakeupDevice(USLOSS_DISK_DEV, 2,USLOSS_DEV_READY, 1)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 22\n");
    assert(P1_WakeupDevice(USLOSS_TERM_DEV, 4,USLOSS_DEV_READY, 1)==P1_INVALID_UNIT);
    USLOSS_Console("P2_Startup 24\n");
    assert(P1_WakeupDevice(-1, 1,USLOSS_DEV_READY, 1)==P1_INVALID_TYPE);
    USLOSS_Console("P2_Startup 26\n");
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}