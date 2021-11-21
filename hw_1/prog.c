#include <stdio.h>
#include <stdlib.h>

#include "ex_1.h"
#include "ex_2.h"
#include "ex_3.h"
#include "ex_4.h"
#include "ex_5.h"

// Helper function to print arrays
void printArray(int *arr, int length) {
	int i;
	printf("{");
	for(i=0; i < length; i++) {
		printf("%d", arr[i]);
		if (i != length-1)
			printf(", ");
	}
	printf("}\n");
}


int main(int argc, char** argv) {
	// ex_1 traingles
	printTriangle(5);
	printTriangle(7);
	printTriangle(21);

	// ex_2 strings
	char *str = instr("Held!", "lo Worl", 3);
	printf("%s\n", str);
	free(str);
	
	char *str2 = instr("Jacob ", "Hull", 6);
	printf("%s\n", str2);
	free(str2);
	
	char *str3 = instr(" Jacob", "Hull", 0);
	printf("%s\n", str2);
	free(str3);

	//ex_3 Arrays
	int arr1[] = {1, 4, 7, 9};
	int arr2[] = {1, 2, 3, 8, 13};
	int *arr3 = merge(arr1, 4, arr2, 5);
	printArray(arr3, 9);	
	free(arr3);
	
	int arr4[] = {1, 2, 3, 4};
	int arr5[] = {5, 6, 7, 8, 9};
	int *arr6 = merge(arr1, 4, arr2, 5);
	printArray(arr6, 9);	
	free(arr6);
	
	// ex_4 pointers and malloc 
	struct Node *head = NULL;
	list_print(head);
	list_insert(&head, 3);
	list_print(head);
	list_insert(&head, 1);
	list_print(head);
	list_insert(&head, 2);
	list_print(head);
	list_insert(&head, 4);
	list_print(head);

	// ex_5 pointers and free
	list_clear(&head);
	list_print(head);
	
	return 0;
}
