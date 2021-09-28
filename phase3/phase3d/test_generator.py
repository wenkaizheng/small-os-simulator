from random import randint

head = """#include <usyscall.h>
#include <libuser.h>
#include <assert.h>
#include <usloss.h>
#include <stdlib.h>
#include <phase3.h>
#include <stdarg.h>
#include <unistd.h>
#include <libdisk.h>

#include "tester.h"
#include "phase3Int.h"

#define PAGES {} // # of pages per process (be sure to try different values)
#define FRAMES {}
#define ITERATIONS 10
#define PAGERS 2 // # of pagers


static char *vmRegion;
static char *names[] = {}; // names of children, add more names to create more children
static int numChildren = sizeof(names) / sizeof(char *);
static int pageSize;

"""

tail = r"""
static int passed = FALSE;

#ifdef DEBUG
static int debugging = 1;
#else
static int debugging = 0;
#endif /* DEBUG */

static void
Debug(char *fmt, ...)
{
    va_list ap;

    if (debugging)
    {
        va_start(ap, fmt);
        USLOSS_VConsole(fmt, ap);
    }
}
static int
Child1(void *arg)
{
    int j;
    int i;
    int rc;
    char *page;
    int pid;
    volatile char *name = (char *)arg;
    Sys_GetPID(&pid);
    Debug("Child (%d) starting.\n", pid);
    int len = name[1] + 1;
    USLOSS_Console("73--------------%s\n", name);
    // Pages should be filled with their page numbers.
    for (j = 0; j < PAGES; j += 2)
    {
        page = vmRegion + j * pageSize;
        Debug("Child %d reading from page %d @ %p\n", pid, j, page);
        for (int k = 0; k < pageSize; k++)
        {
            assert(page[k] == '\0');
        }
    }
    int n = (int)random();
    for (i = 0; i < ITERATIONS; i += 2)
    {
        for (j = PAGES - 1; j >= 0; j--)
        {
            rc = Sys_Sleep(2);
            assert(rc == P1_SUCCESS);
            page = vmRegion + j * pageSize;
            //Debug("Child \"%s\" (%d) writing to page %d @ %p\n", name, pid, j, page);
            for (int k = pageSize - 1; k > 0; k -- )
            {
                if (len == 1)
                {
                    page[k] = (char)((*name) + n % 9);
                }
                else
                {
                    page[k] = (char)((*name) - 16);
                }
            }
        }
        for (j = PAGES - 1; j >= 0; j--)
        {
            rc = Sys_Sleep(2);
            assert(rc == P1_SUCCESS);
            page = vmRegion + j * pageSize;
            // Debug("Child \"%s\" (%d) reading from page %d @ %p\n", name, pid, j, page);
            for (int k = pageSize - 1; k > 0; k--)
            {
                if (len == 1)
                {
                    assert(page[k] == (*name) + n % 9);
                }
                else
                {
                    assert(page[k] == (*name) - 16);
                }
            }
        }
        //  Debug("Child \"%s\" (%d) done.\n", name, pid);
      
    }
      Debug("Sub Childs done.\n");
        return 0;
}
    static int
    Child(void *arg)
    {
        volatile char *name = (char *)arg;
        int i, j;
        char *page;
        int rc;
        int pid;

        Sys_GetPID(&pid);
        Debug("Child \"%s\" (%d) starting.\n", name, pid);

        // The first time a page is read it should be full of zeros.
        for (j = 0; j < PAGES; j += 1)
        {
            page = vmRegion + j * pageSize;
            Debug("Child \"%s\" (%d) reading zeros from page %d @ %p\n", name, pid, j, page);
            for (int k = 0; k < pageSize; k++)
            {
                assert(page[k] == '\0');
            }
        }
        int n = (int)random();
        for (i = 0; i < ITERATIONS; i += 1)
        {
            for (j = PAGES - 1; j >= 0; j--)
            {
                rc = Sys_Sleep(2);
                assert(rc == P1_SUCCESS);
                page = vmRegion + j * pageSize;
                Debug("Child \"%s\" (%d) writing to page %d @ %p\n", name, pid, j, page);
                for (int k = pageSize - 1; k > 0; k -= 1)
                {
                    page[k] = (char)((*name) + n % 3);
                }
            }
            for (j = PAGES - 1; j >= 0; j--)
            {
                rc = Sys_Sleep(2);
                assert(rc == P1_SUCCESS);
                page = vmRegion + j * pageSize;
                Debug("Child \"%s\" (%d) reading from page %d @ %p\n", name, pid, j, page);
                for (int k = pageSize - 1; k > 0; k -= 1)
                {
                    if(page[k] != *name + n % 3){
                        Debug("when j = %d page[k] %d should be %d\n", j, page[k], *name + n % 3);continue;
                    }
                    assert(page[k] == *name + n % 3);
                }
            }
        }
        Debug("Child \"%s\" (%d) done.\n", name, pid);

        return 0;
    }

    int P4_Startup(void *arg)
    {
        srandom(2147444228);
        int i;
        int rc;
        int pid;
        int status;

        Debug("P4_Startup starting.\n");
        rc = Sys_VmInit(PAGES, PAGES, FRAMES, PAGERS, (void **)&vmRegion);
        TEST(rc, P1_SUCCESS);

        pageSize = USLOSS_MmuPageSize();
        for (i = 0; i < numChildren; i++)
        {
            rc = Sys_Spawn(names[i], Child, (void *)names[i], USLOSS_MIN_STACK * 4, 5, &pid);
            assert(rc == P1_SUCCESS);
        }
        char buffer[81];
        memset(buffer, 0, 81);
        for (i = 0; i < numChildren; i++)
        {
            rc = Sys_Wait(&pid, &status);
            if (i % 2 == 0)
            {
                strcat(buffer, names[i]);
                strcat(buffer, " additional_child");
                USLOSS_Console("194th %s\n", buffer);
                rc = Sys_Spawn(buffer, Child1, buffer, USLOSS_MIN_STACK * 4, 1, &pid);
                assert(rc==P1_SUCCESS);
                memset(buffer, 0, 81);
                rc = Sys_Wait(&pid, &status);
                assert(rc == P1_SUCCESS);
                assert(status == 0);
            }
            assert(rc == P1_SUCCESS);
            TEST(status, 0);
        }
        Debug("Children terminated\n");
        Sys_VmShutdown();
        PASSED();
        return 0;
    }

    void test_setup(int argc, char **argv)
    {
        DeleteAllDisks();
        int rc = Disk_Create(NULL, P3_SWAP_DISK, numChildren * PAGES);
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




"""

N = 40


def generate_one():
    number_of_process = randint(5, 30)
    number_of_pages = randint(1, 10)
    number_of_frame = randint(2, number_of_process + number_of_pages)
    return tuple([number_of_pages, number_of_frame, number_of_process])


def generate_N():
    cases = set()
    i = 0
    while i < N:
        t = generate_one()
        if t not in cases:
            cases.add(t)
            i += 1
    return cases




def write_to_file(cases):
    cases = list(cases)
    letters = "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z "
    letters += letters.lower()
    letters = letters.strip().split()
    for i in range(N):
        t = cases[i]
        process = letters[:t[2]]
        process = ",".join(["\""+i+"\"" for i in process])
        process = "{" + process + "}"
        with open("test_random{}.c".format(i), "w+") as f:
            f.write(head.format(t[0], t[1], process))
            f.write(tail)

write_to_file(generate_N())
