#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include "usloss.h"



void
startup(int argc, char **argv)
{
    P1ContextInit();
    assert(P1DisableInterrupts()==0);
    assert((USLOSS_PsrGet() & 0x2) >>1==0);

    P1EnableInterrupts();
    assert((USLOSS_PsrGet() & 0x2) >>1==1);

    assert(P1DisableInterrupts()==1);
    assert((USLOSS_PsrGet() & 0x2) >>1==0);
    USLOSS_Halt(0);
   
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}