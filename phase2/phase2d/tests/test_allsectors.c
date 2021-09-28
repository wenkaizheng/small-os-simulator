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

//define MSG "This is a test."

int P3_Startup(void *arg) {
    char buffer[USLOSS_DISK_SECTOR_SIZE*160];
    int i;
    char MSG[USLOSS_DISK_SECTOR_SIZE * 160];
    MSG[USLOSS_DISK_SECTOR_SIZE * 160-1] = '\0';
    for(i=0;i<USLOSS_DISK_SECTOR_SIZE * 160-1;i++){
        MSG[i] ='a'+(i%26);
    }
    strncpy(buffer, MSG, sizeof(buffer));

    USLOSS_Console("Write to the disk.\n");
    int rc = Sys_DiskWrite(buffer, 0, 0, 160, 0);
    USLOSS_Console("Verify that the disk write was successful.\n");
    assert(rc == P1_SUCCESS);
    USLOSS_Console("Wrote \"%s\".\n", buffer);

    bzero(buffer, sizeof(buffer));
    USLOSS_Console("Read from the disk.\n");
    rc = Sys_DiskRead(buffer, 0, 0, 160, 0);
    USLOSS_Console("Verify that the disk read was successful.\n");
    assert(rc == P1_SUCCESS);
    assert(strcmp(MSG, buffer)==0);
    USLOSS_Console("Read \"%s\".\n", buffer);
    return 11;
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