/*
   Info:
   Group Member: Wenkai Zheng & Jiacheng Yang
   NetId:        wenkaizheng & jiachengyang
   Submission type: Group
*/

/*
 * phase3b.c
 *
 */

#include <assert.h>
#include <phase1.h>
#include <phase2.h>
#include <usloss.h>
#include <string.h>
#include <libuser.h>
#include <stdlib.h>
#include "phase3Int.h"

void
P3PageFaultHandler(int type, void *arg) {
    /*******************

    if the cause is USLOSS_MMU_FAULT (USLOSS_MmuGetCause)
        if the process does not have a page table  (P3PageTableGet)
            print error message
            USLOSS_Halt(1)
        else
            determine which page suffered the fault (USLOSS_MmuPageSize)
            update the page's PTE to map page x to frame x
            set the PTE to be read-write and incore
            update the page table in the MMU (USLOSS_MmuSetPageTable)
    else
        print error message
        USLOSS_Halt(1)
    *********************/
    assert(type == USLOSS_MMU_INT);
    int rc = USLOSS_MmuGetCause();
    int pid = P1_GetPid();
    unsigned int offset = (unsigned int) arg;
    USLOSS_PTE *table = NULL;
    if (rc == USLOSS_MMU_FAULT) {
        rc = P3PageTableGet(pid, &table);
        assert(rc == P1_SUCCESS); //todo might need a if branch???
        if (table == NULL) {
            USLOSS_Console("PAGE FAULT!!! PID %d offset 0x%x\n", pid, offset);
            USLOSS_Halt(1);
        } else {
            unsigned int errorPageIndex = offset / USLOSS_MmuPageSize();
            //todo do we need to check if the error page is beyond the size
            // of page table
            (table + errorPageIndex)->read = 1;
            (table + errorPageIndex)->write = 1;
            (table + errorPageIndex)->incore = 1;
            (table + errorPageIndex)->frame = errorPageIndex;
            rc = USLOSS_MmuSetPageTable(table);
            assert(rc == USLOSS_MMU_OK);
        }
    } else {
        USLOSS_Console("PAGE FAULT!!! PID %d offset 0x%x\n", pid, offset);
        USLOSS_Halt(1);
    }

}

USLOSS_PTE *
P3PageTableAllocateEmpty(int pages) {
    USLOSS_PTE *table = NULL;
    // allocate and initialize an empty page table

    table = malloc(pages * sizeof(USLOSS_PTE));
    if (table == NULL) {
        return NULL;
    }
    unsigned int i;
    for (i = 0; i < pages; i++) {
        (table + i)->read = 0;
        (table + i)->write = 0;
        (table + i)->incore = 0;
        (table + i)->frame = 0;
    }
    return table;
}
