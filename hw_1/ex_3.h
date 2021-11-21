#ifndef EX_3_H
#define EX_3_H

#include <stdlib.h>

int* merge(int *arr1, int arr1Length, int *arr2, int arr2Length) {
	// Make space for an array the size of the two together.
	int sizeOfBigArray = arr1Length + arr2Length;
	int *bigArray = malloc(sizeOfBigArray * sizeof(int));
	
	// Copy arr1 to the bigger array
	for (int i = 0; i < arr1Length; i++) {
		bigArray[i] = arr1[i];
	}
	
	// Copy arr2 to the end of the bigger array
	for (int j = 0; j < arr2Length; j++) {
		bigArray[j + arr1Length] = arr2[j];
	}

	// Sort the big array using a simple double for loop technique
	for (int i = 0; i < sizeOfBigArray - 1; i++) {
		for (int j = i + 1; j < sizeOfBigArray; j++) {
			if (bigArray[i] > bigArray[j]) {
				int temp = bigArray[j];
				bigArray[j] = bigArray[i];
				bigArray[i] = temp;
			}
		}
	}

	// Return the pointer to the merged array
	return bigArray;
}

#endif /* EX_3_H */
