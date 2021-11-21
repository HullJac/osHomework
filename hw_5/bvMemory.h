#include <stdlib.h>
#include <stdio.h>

// Sruct to signify the filled space
struct node {
    struct node* next;
    int size;
    void* start;
    void* end;
};

// Helper function to create nodes easily
struct node* createNewNode(struct node* n, int s, void* st) {
    struct node* newNode = (struct node*) malloc(sizeof(struct node));
    
    newNode->next = n;
    newNode->size = s;
    newNode->start = st;
    newNode->end = newNode->start + s - 1;
    
    return newNode;
}

// Global Variables
int first = 1; // Indicator if we have malloced or not yet
int empty = 1; // Indicator if the malloced space is empty
void* heap; // Pointer to the start of the malloced space
void* endHeap; // Pointer to the end of the heap
struct node* head; // First node in space

// If bestFit == 1 do best fit, if 0 do firstFit
// size here represents bytes
void* bvMalloc(int size, int bestFit) {
    // Checking to see if we need to actually call malloc
    if (first == 1) {
        heap = malloc(1000);
        endHeap = heap + 1000;
        first = 0;
    }
    
    // If there is nothing in the space yet
    if (empty == 1) {
        if (size <= endHeap - heap) {
            empty = 0;
            struct node* newNode = createNewNode(NULL, size, heap);
            head = newNode;
            return head -> start;
        }
        else {
            return 0;
        }
    }

    else {
        // BestFit
        if (bestFit == 1) {
            // How tight the fit is for the node
            int ifStatement = 1;
            int tightness;
            void* bestAddress;
            struct node* curr = head;
            struct node* prev = head;
            // Check for if there is space before the head and the front of the heap
            if (size <= ((curr->start) - heap)) {
                bestAddress = heap;
                tightness = (curr->start) - heap;
                ifStatement = 0;
            }
            // Go through everything and see if there is a better spot in between two nodes
            while (curr -> next != NULL) {
                curr = curr -> next;
                if (size <= ((curr->start-1) - (prev->end))) {
                    if (ifStatement == 1) {
                        ifStatement = 0;
                        tightness = (curr->start-1) - (prev->end);
                        bestAddress = (prev->end) + 1;
                    }
                    else {
                        int newTight = (curr->start-1) - (prev->end);
                        if (newTight < tightness) {
                            tightness = newTight;
                            bestAddress = (prev->end) + 1;
                        }
                    }
                    prev = curr;
                }
                else {
                    prev = prev -> next;
                }
                
            }
            // Check if the best spot is after the last node
            if (size <= ((endHeap -1) - (curr->end))) {
                // If nothing else has been found to fit and we are at the end
                // just create the node and put it at the end then
                if (ifStatement == 1) {
                    struct node* newNode = createNewNode(NULL, size, ((curr->end) + 1));
                    curr -> next = newNode;
                    return newNode -> start;
                }
                // Otherwise check tightness and continue on
                else {
                    int newTight = (endHeap - 1) - (curr->end);
                    if (newTight < tightness) {
                        tightness = newTight;
                        bestAddress = (curr->end) + 1;
                    }
                }
            }
            // Go through and add the node at the saved address
            if (ifStatement == 0) {
                // If the newNode goes before head
                if (bestAddress == heap) {
                    struct node* newNode = createNewNode(head, size, heap);
                    head = newNode;
                    return heap;
                }
                // If the newNode goes between two nodes
                else {
                    curr = head;
                    prev = head;
                    while ((curr->next) !=  NULL) {
                        curr = curr -> next;
                        // If we have found the address we saved before
                        if (((prev->end) + 1) == bestAddress) {
                            struct node* newNode = createNewNode(curr, size, ((prev->end) +1));
                            prev -> next = newNode;
                            return ((prev->end) + 1);
                        }
                        // Otherewise advance
                        else {
                            prev = curr;
                        }
                    }
                }
            }
            // Otherwise just return zero if we dont add anything
            else{
                return 0;
            }
        }
        // FirstFit
        else {
            struct node* curr = head;
            struct node* prev = head;
            // Check if the firstFit is before head
            if (size <= ((curr->start) - heap)) {
                struct node* newNode = createNewNode(curr, size, heap);
                head = newNode;
                return newNode -> start;
            }
            while (curr -> next != NULL) {
                curr = curr -> next;
                // If we can fit the block between the two nodes
                if (size <= ((curr->start-1) - (prev->end))) {
                    struct node* newNode = createNewNode(curr, size, (prev->end + 1));
                    prev -> next = newNode;
                    return newNode -> start;
                }
                else {
                    prev = prev -> next;
                }
            }
            // Otherwise check to see if we can fit it in the end
            if (size <= ((endHeap -1) - (curr->end))) {
                struct node* newNode = createNewNode(NULL, size, ((curr->end) + 1));
                curr -> next = newNode;
                return newNode -> start;
            }
            else {
                return 0;
            }
        }
    }
    return 0;
}

// Virtual free function that takes the address that is given when the user called bvMalloc
void bvFree(void* address) {
    int second = 1;
    struct node* curr = head;
    struct node* prev = head;
    // Find the node that corresponds to the address passed
    while (curr->start != address) {
        curr = curr -> next;
        if (second == 0) {
            prev = prev -> next;
        }
        else {
            second = 0;
        }
    }
    // Check if we are deleting the first thing 
    if (second == 1) {
        // Check if it is the only space occupied
        if (curr -> next == NULL) {
            empty = 1;
            head = NULL;
            free(curr);
        }
        else {
            head = curr -> next;
            free(curr);
        }
    }
    else {
        // Assign the previous thing to point to the next
        prev -> next = curr -> next;
        free(curr);
    }
}


void printNodes(struct node* h) {
    struct node* curr = h;
    int i = 0;
    while (curr != NULL) {
        printf("%d: %p\n",i, curr->start);
        //printf("%d: %p\n",i, curr->end);
        curr = curr->next;
        i++;
    }
}
