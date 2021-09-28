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
    int sector, track, disk;
    int rc = Sys_DiskSize(1, &sector, &track, &disk);
    assert(rc == P1_SUCCESS);
    assert(sector == USLOSS_DISK_SECTOR_SIZE);
    assert(track == USLOSS_DISK_TRACK_SIZE);
    assert(disk == 78);
    rc = Sys_DiskSize(0, &sector, &track, &disk);
    assert(rc == P1_INVALID_UNIT);
    return 11;
}

void test_setup(int argc, char **argv) {
    int rc;

    rc = Disk_Create(NULL, 1, 78);
    assert(rc == 0);
}

void test_cleanup(int argc, char **argv) {
    DeleteAllDisks();
    if (passed) {
        USLOSS_Console("TEST PASSED.\n");
    }
}