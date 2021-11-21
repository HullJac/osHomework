#ifndef EX_5_H
#define EX_5_H

#include "linkedList.h"

void list_clear(struct Node** head) {
   struct Node* curr = *head;
   struct Node* Next;

   while (curr != NULL) {
       Next = curr->next;
       free(curr);
       curr = Next;
   }

   *head = NULL;
}

#endif /* EX_5_H */
