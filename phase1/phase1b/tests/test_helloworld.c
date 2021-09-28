#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int static time = -1;
int static World(void *arg);

int static Hello(void *arg1)
{
    USLOSS_Console("CCCCCCCNNNNNNNNMMMMMMM");
    USLOSS_Console("%d", time);
    USLOSS_Console(" ");
    USLOSS_Console("Hello");
    USLOSS_Console(" ");
    if (time < 3)
    {
        char str[100];
        sprintf(str, "%d", time);
        int pid, rc;
        char name[100];
        strcpy(name, "World");
        strcat(name, str);
        
        time++;
        rc = P1_Fork(name, World, "", USLOSS_MIN_STACK, 5 - time, 0, &pid);
        int status, pid2;
        P1GetChildStatus(0, &pid2, &status);
        assert(pid2 == pid);
        assert(status == 0);
    }
    return 0;
}
int static World(void *arg)
{

    USLOSS_Console("World");
    int j;
    for (j = 0; j < time; j++)
    {
        USLOSS_Console("!");
    }
    USLOSS_Console("\n");
    if (time < 3)
    {
        char str[100];
        sprintf(str, "%d", time);
        int pid, rc;
        char name[100];
        strcpy(name, "Hallo");
        strcat(name, str);
        time++;
        rc = P1_Fork(name, Hello, "", USLOSS_MIN_STACK, 5 - time, 0, &pid);
        int status, pid2;
        P1GetChildStatus(0, &pid2, &status);
        assert(pid2 == pid);
        assert(status == 0);
    }
    return 0;
}
void startup(int argc, char **argv)
{
    int pid;
    int rc;
    P1ProcInit();
    USLOSS_Console("startup\n");
    rc = P1_Fork("123", Hello, "", USLOSS_MIN_STACK, 6, 0, &pid);
    assert(rc == P1_SUCCESS);
    // P1_Fork should not return
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}