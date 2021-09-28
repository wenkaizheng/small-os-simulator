#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <usloss.h>
#include <phase1.h>
#include <assert.h>
#include <libuser.h>
#include <libdisk.h>

#include "tester.h"
#include "phase2Int.h"

static int passed = FALSE;

#define MSG "This is a test."

int P3_Startup(void *arg) {
    char buffer[USLOSS_DISK_SECTOR_SIZE];
    strncpy(buffer, MSG, sizeof(buffer));

    USLOSS_Console("Write to the disk.\n");
    int rc = Sys_DiskWrite(buffer, 0, 0, 1, 2);
    assert(rc == P1_INVALID_UNIT);

    rc = Sys_DiskWrite(NULL, 0, 0, 1, 0);
    assert(rc ==  P2_NULL_ADDRESS);
    
    rc = Sys_DiskWrite(buffer, 0, 0, 1, 1);
    assert(rc == P1_INVALID_UNIT);

    rc =  Sys_DiskWrite(buffer, 0, -1, 1, 0);
    assert(rc == P2_INVALID_FIRST);

    rc =  Sys_DiskWrite(buffer, 0, 17, 1, 0);
    assert(rc == P2_INVALID_FIRST);

    rc = Sys_DiskWrite(buffer, 11, 0, 1, 0);
    assert(rc == P2_INVALID_TRACK);

    rc = Sys_DiskWrite(buffer, 0, 0, 161, 0);
    assert(rc == P2_INVALID_SECTORS);
    int disk ;
    USLOSS_Console("49th\n");
    int sector,track;
    rc = Sys_DiskSize(2, &sector, &track, &disk);
    USLOSS_Console("51th\n");
    assert(rc ==P1_INVALID_UNIT);

    rc = Sys_DiskSize(0,&sector,&track,&disk);
    //assert(rc ==P1_SUCCESS);
    USLOSS_Console("57th %d %d\n",rc,disk);
    assert(disk ==10);


    // read is from here and above is write
    bzero(buffer, sizeof(buffer));
    USLOSS_Console("Read from the disk.\n");

    rc = Sys_DiskRead(buffer, 0, 0, 1, 2);
    assert(rc == P1_INVALID_UNIT);

    rc = Sys_DiskRead(NULL, 0, 0, 1, 0);
    assert(rc ==  P2_NULL_ADDRESS);

    rc = Sys_DiskRead(buffer, 0, 0, 1, 1);
    assert(rc == P1_INVALID_UNIT);

    rc =  Sys_DiskRead(buffer, 0, -1, 1, 0);
    assert(rc == P2_INVALID_FIRST);

    rc =  Sys_DiskRead(buffer, 0, 17, 1, 0);
    assert(rc == P2_INVALID_FIRST);

    rc = Sys_DiskRead(buffer, 11, 0, 1, 0);
    assert(rc == P2_INVALID_TRACK);

    rc = Sys_DiskRead(buffer, 0, 0, 161, 0);
    assert(rc == P2_INVALID_SECTORS);

    USLOSS_Console("Verify that the disk read.\n");
  
    return 11;
}
int P2_Startup(void *arg)
{
    int rc, waitPid, status, p3Pid;

    P2ClockInit();
    P2DiskInit();
    rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4*USLOSS_MIN_STACK, 3, &p3Pid);
    assert(rc== P1_SUCCESS);
    rc = P2_Wait(&waitPid, &status);
    assert(rc== P1_SUCCESS);
    assert(waitPid== p3Pid);
    assert(status== 11);
    P2DiskShutdown();
    P2ClockShutdown();
    USLOSS_Console("You passed all the tests! Treat yourself to a cookie!\n");
    PASSED();
    return 0;
}


void test_setup(int argc, char **argv) {
    int rc;

    rc = Disk_Create(NULL, 0, 10);
    assert(rc == 0);
}

void test_cleanup(int argc, char **argv) {
    DeleteAllDisks();
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}