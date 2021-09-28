#include <stdio.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase1Int.h>
#include <assert.h>

// Hello(): test context switch
static int cids[2];
void
Hello()
{
        USLOSS_Console("In Hello, switching to World.\n");
	int retval = P1ContextSwitch(cids[1]);
	assert(retval == P1_SUCCESS);
	USLOSS_Console("Back in Hello.\n");
	USLOSS_Halt(0);
}

void World(){
	USLOSS_Console("In World, switching to Hello.\n");
	P1ContextSwitch(cids[0]);
}
void
startup(int argc, char **argv)
{

    int retval;
    P1ContextInit();
    retval = P1ContextCreate(Hello, NULL, USLOSS_MIN_STACK, &cids[0]);
    assert(retval == P1_SUCCESS);

    retval = P1ContextCreate(World, NULL, USLOSS_MIN_STACK, &cids[1]);
    assert(retval == P1_SUCCESS);
    retval = P1ContextSwitch(cids[0]);

    // should not get to here
    USLOSS_Console("This line should not be here.\n");
}

// Do not modify anything below this line.

void
finish(int argc, char **argv)
{
    USLOSS_Console("Goodbye.\n");
}

void
test_setup(int argc, char **argv)
{
    // Do nothing.
}

void
test_cleanup(int argc, char **argv)
{
    // Do nothing.
}

