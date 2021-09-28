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

//#define MSG "This is a test."
void makeString(char* buffer, char* MSG,int num){
    srand(num);
    int inc = (random() % 11) * 13 + (random() % 3)+num;
   // USLOSS_Console("25-------- %d\n",inc);
    int i;
    for (i = 0; i < USLOSS_DISK_SECTOR_SIZE * 16 - 1; i++)
    {
        MSG[i] = 'a' + ((inc+i) % 26);
    }
    strncpy(buffer, MSG, USLOSS_DISK_SECTOR_SIZE * 16);
}
int P3_Disk1Startup(void *arg)
{
    char buffer[USLOSS_DISK_SECTOR_SIZE * 16];
    buffer[USLOSS_DISK_SECTOR_SIZE * 16 - 1] = '\0';
    //  char buffer[USLOSS_DISK_SECTOR_SIZE];
    char MSG[USLOSS_DISK_SECTOR_SIZE * 16 ];
    MSG[USLOSS_DISK_SECTOR_SIZE * 16-1] = '\0';
   
   // strncpy(buffer, MSG, sizeof(buffer));
    //USLOSS_Console("%d\n",strcmp(MSG,buffer));
    assert(strcmp(buffer, MSG) == 0);
    // USLOSS_Console("%ld\n",sizeof(buffer));
    int i;
    for (i = 0; i < 1000; i++)
    {
        makeString(buffer,MSG,i);
        USLOSS_Console("%d %d\n",strlen(buffer),strlen(MSG));
        assert(strcmp(buffer, MSG) == 0);
        USLOSS_Console("Write to the disk1.\n");
        int rc = Sys_DiskWrite(buffer, i, 0, 16, 1);
        USLOSS_Console("Disk1 Verify that the disk write was successful.\n");
        assert(rc == P1_SUCCESS);
        USLOSS_Console("Disk1 Wrote %dth time \"%s\".\n",i, buffer);
        //assert(strcmp(buffer,MSG)==0);
        bzero(buffer, sizeof(buffer));
        USLOSS_Console("Read from the disk1.\n");
        rc = Sys_DiskRead(buffer, i, 0, 16, 1);
        USLOSS_Console("Disk1 Verify that the disk read was successful.\n");
        assert(rc == P1_SUCCESS);
        assert(strcmp(buffer, MSG) == 0);
        USLOSS_Console("Disk1 Read %dth time \"%s\".\n", i,buffer);
    }
    //USLOSS_Console("Write \"%s\".\n",MSG);
    return 12;
}
int P3_Disk0Startup(void *arg)
{
    char buffer[USLOSS_DISK_SECTOR_SIZE * 16];
    buffer[USLOSS_DISK_SECTOR_SIZE * 16 - 1] = '\0';
    //  char buffer[USLOSS_DISK_SECTOR_SIZE];
    char MSG[USLOSS_DISK_SECTOR_SIZE * 16];
    MSG[USLOSS_DISK_SECTOR_SIZE * 16-1] = '\0';
    int i;
    
    //USLOSS_Console("%d\n",strcmp(MSG,buffer));
    assert(strcmp(buffer, MSG) == 0);
    // USLOSS_Console("%ld\n",sizeof(buffer));
    for (i = 0; i < 1000; i++)
    {
        makeString(buffer,MSG,i+10);
        USLOSS_Console("%d %d\n",strlen(buffer),strlen(MSG));
        assert(strcmp(buffer, MSG) == 0);
        USLOSS_Console("Write to the disk0.\n");
        int rc = Sys_DiskWrite(buffer, i, 0, 16, 0);
        USLOSS_Console("Disk0 Verify that the disk write was successful.\n");
        assert(rc == P1_SUCCESS);
        USLOSS_Console("Disk0 Wrote %dth time \"%s\".\n", i,buffer);
        //assert(strcmp(buffer,MSG)==0);
        bzero(buffer, sizeof(buffer));
        USLOSS_Console("Read from the disk0.\n");
        rc = Sys_DiskRead(buffer, i, 0, 16, 0);
        USLOSS_Console("Disk0 Verify that the disk read was successful.\n");
        assert(rc == P1_SUCCESS);
        assert(strcmp(buffer, MSG) == 0);
        USLOSS_Console("Disk0 Read %dth time \"%s\".\n", i,buffer);
    }
    //USLOSS_Console("Write \"%s\".\n",MSG);
    return 11;
}
int P2_Startup(void *arg)
{
    int rc, waitPid, status, p3Pid1, p3Pid2;

    P2ClockInit();
    P2DiskInit();
    rc = P2_Spawn("P3_Disk0Startup", P3_Disk0Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &p3Pid1);
    rc = P2_Spawn("P3_Disk1Startup", P3_Disk1Startup, NULL, 4 * USLOSS_MIN_STACK, 3, &p3Pid2);
    assert(rc ==P1_SUCCESS);

    rc = P2_Wait(&waitPid, &status);
    assert(rc ==P1_SUCCESS);
    assert(waitPid== p3Pid1);
    assert(status== 11);

    rc = P2_Wait(&waitPid, &status);
    assert(rc == P1_SUCCESS);
    assert(waitPid== p3Pid2);
    assert(status== 12);

    P2DiskShutdown();
    P2ClockShutdown();

    USLOSS_Console("You passed all the tests! Treat yourself to a cookie!\n");
    PASSED();
    return 0;
}

void test_setup(int argc, char **argv)
{
    int rc;

    rc = Disk_Create(NULL, 0, 1000);

    assert(rc == 0);

   rc = Disk_Create(NULL, 1, 1000);

    assert(rc == 0);
}

void test_cleanup(int argc, char **argv)
{
    DeleteAllDisks();
    if (passed)
    {
        USLOSS_Console("TEST PASSED.\n");
    }
}