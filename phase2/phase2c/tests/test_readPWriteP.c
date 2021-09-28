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
#define msg "This is another test"
int P3_Startup(void *arg) {
    char buffer[USLOSS_DISK_SECTOR_SIZE];
    strncpy(buffer, MSG, sizeof(buffer));

    USLOSS_Console("Write to the disk0.\n");
    int rc = Sys_DiskWrite(buffer, 0, 0, 1, 0);
    USLOSS_Console("Verify that the disk0 write was successful.\n");
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Disk 0 Wrote \"%s\".\n", buffer);
    
    bzero(buffer, sizeof(buffer));
    strncpy(buffer, msg, sizeof(buffer));

    USLOSS_Console("Write to the disk1.\n");
    rc = Sys_DiskWrite(buffer, 0, 3, 1, 0);
    USLOSS_Console("Verify that the disk1 write was successful.\n");
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Disk 0 Wrote \"%s\".\n", buffer);

    return 11;
}

int P3_StartupDisk2(void *arg) {
  
    char buffer[USLOSS_DISK_SECTOR_SIZE];
    bzero(buffer, sizeof(buffer));
    USLOSS_Console("Read from the disk0.\n");
    int rc = Sys_DiskRead(buffer, 0, 0, 1, 0);
    USLOSS_Console("Verify that the disk0 read was successful.\n");
    assert(rc == P1_SUCCESS);
    assert(strcmp(MSG, buffer)== 0);
    USLOSS_Console("Disk 0 Read \"%s\".\n", buffer);
   

    bzero(buffer, sizeof(buffer));
    USLOSS_Console("Read from the disk1.\n");
    rc = Sys_DiskRead(buffer, 0, 3, 1, 0);
    USLOSS_Console("Verify that the disk1 read was successful.\n");
    assert(rc == P1_SUCCESS);
    assert(strcmp(msg, buffer)== 0);
    USLOSS_Console("Disk 1 Read \"%s\".\n", buffer);
    return 12;
}
int P2_Startup(void *arg)
{
    int rc, waitPid, status, p3Pid,p3Pids;

    P2ClockInit();
    P2DiskInit();
    rc = P2_Spawn("P3_Startup", P3_Startup, NULL, 4*USLOSS_MIN_STACK, 3, &p3Pid);
    assert(rc==P1_SUCCESS);
    
    rc = P2_Spawn("P3_StartupDisk2", P3_StartupDisk2, NULL, 4*USLOSS_MIN_STACK, 3, &p3Pids);
    assert(rc== P1_SUCCESS);

    rc = P2_Wait(&waitPid, &status);
    assert(rc == P1_SUCCESS);
    assert(waitPid ==p3Pid);
    assert(status== 11);

    rc = P2_Wait(&waitPid, &status);
    assert(rc ==P1_SUCCESS);
    assert(waitPid== p3Pids);
    assert(status==12);

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

    rc =Disk_Create(NULL,1,10);
    assert(rc ==0);
}

void test_cleanup(int argc, char **argv) {
    DeleteAllDisks();
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}