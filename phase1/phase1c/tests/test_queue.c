#include <phase1.h>
#include <phase1Int.h>
#include <assert.h>
#include <stdlib.h>
typedef struct diskRequestQueueNode
{
    int pid;
    struct diskRequestQueueNode *next;
} queueNode;

diskRequestQueueNode *diskRequestQueueInsert(int pid, diskRequestQueueNode *node) {
    diskRequestQueueNode *head = malloc(sizeof(diskRequestQueueNode));
    head->next = NULL;
    head->pid = pid;
    if (node == NULL) {
        return head;
    } else {
        diskRequestQueueNode *walker = node;
        while (walker->next != NULL) {
            walker = walker->next;
        }
        walker->next = head;
        return node;
    }
}

int remove(diskRequestQueueNode **node) {
    if (*node == NULL) {
        return -1;
    } else {
        diskRequestQueueNode *head = *node;
        int pid = head->pid;
        *node = (*node)->next;
        free(head);
        return pid;
    }
}

void
startup(int argc, char **argv)
{
    diskRequestQueueNode* node = NULL;
    node = diskRequestQueueInsert(0, node);
    node = diskRequestQueueInsert(1, node);
    // 0 1
    int node1 = remove(&node);
    // 1
    //USLOSS_Console("%d\n",node1);
    assert(node1 == 0);
    int node2 = remove(&node);
    // remove 1 empty and we can get NULL
    assert(node2 == 1);
    assert(node == NULL);
    int node3 = remove( &node);
    assert(node3 == -1);
    assert(node == NULL);
    

    node = diskRequestQueueInsert(2, node);
    USLOSS_Console("%d\n",node->pid);
    node = diskRequestQueueInsert(3, node);
    USLOSS_Console("%d\n",node->pid);
    USLOSS_Console("%d\n",node->next->pid);
    node = diskRequestQueueInsert(4, node);
    USLOSS_Console("%d\n",node->pid);
    USLOSS_Console("%d\n",node->next->pid);
    USLOSS_Console("%d\n",node->next->next->pid);
    int node4 = remove(&node);
    assert( node4 ==2);
    node = diskRequestQueueInsert(5, node);
    int node5 = remove(&node);
    assert(node5 ==3);
    diskRequestQueueNode* copy = node;
    while(copy !=NULL){
        USLOSS_Console("%d ",copy->pid);
        copy = copy -> next;
    }
    USLOSS_Console("abc\n");
    int node6 = remove(&node);
    assert(node6 == 4);
    int node7 = remove(&node);
    assert(node7 == 5);

    USLOSS_Halt(0);
}

void test_setup(int argc, char **argv) {}

void test_cleanup(int argc, char **argv) {}

void finish(int argc, char **argv) {}