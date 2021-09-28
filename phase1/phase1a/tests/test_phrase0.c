//
// Created by kv9699 on 9/13/19.
//

#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>

int hallo_cid, world_cid;





void Hallo(void *arg){
    int i = 1;
    for(;i<=10;i++){
        USLOSS_Console("%d Hello ", i);
        P1ContextSwitch(world_cid);
    }
    USLOSS_Halt(0);//terminate
}


void World(void *arg){
    int i=1;
    int j;
    for(;i<=10;i++){
        USLOSS_Console("World");
        j=0;
        for(;j<i;j++){ // print !*n
            USLOSS_Console("!");
        }
        USLOSS_Console("\n");
        P1ContextSwitch(hallo_cid);
    }
    USLOSS_Halt(0);//terminate
}

void
startup(int argc, char **argv)
{
    int rc;
    P1ContextInit();

    rc = P1ContextCreate(Hallo, 0, USLOSS_MIN_STACK, &hallo_cid);
    assert(rc == P1_SUCCESS);

    rc = P1ContextCreate(World, 0, USLOSS_MIN_STACK, &world_cid);
    assert(rc == P1_SUCCESS);


    rc = P1ContextSwitch(hallo_cid);
    // should not return
    assert(rc == P1_SUCCESS);
    assert(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {
    P1ContextFree(hallo_cid);
    P1ContextFree(world_cid);
}