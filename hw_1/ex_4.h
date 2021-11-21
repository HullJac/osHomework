#ifndef EX_4_H
#define EX_4_H

#include "linkedList.h"
#include <stdlib.h>

void list_insert(struct Node** head, int value) {
	struct Node* newNode = malloc(sizeof(struct Node));
	newNode -> data = value;
	if ((*head) == NULL) {
		newNode -> next = NULL;
		*head = newNode;
	}
	else {
		struct Node* curr = *head;
		struct Node* prev;
		int first = 1;
		int done = 0;
		// Find where the node needs to go
		while (curr != NULL) {
			if (curr -> data > value) {
				// If the node needs to be the new head
				if (first == 1) {
					done = 1;
					newNode -> next = *head;
					*head = newNode;
				}
				// If the node is in the the middle 
				else {
					done = 1;
					prev -> next = newNode;
					newNode -> next = curr;
				}
			}
			prev = curr;
			curr = curr -> next;
			first = 0;
		}
		// If the node needs to go at the end
		if (done == 0) {
			newNode -> next = NULL;
			prev -> next = newNode;
		}
		
	}
}

#endif /* EX_4_H */
