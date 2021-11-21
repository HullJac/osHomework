#ifndef EX_1_H
#define EX_1_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void printTriangle(int baseLength) {
	int height = ceil(baseLength/2.0);
	for (int j = 1; j <= baseLength; j+=2) {
		for (int i = height; i > 1; i--) {
			printf(" ");
		}
		for (int numStars = j; numStars > 0; numStars--) {
			printf("#");
		}
		printf("\n");
		height-=1;
	}
}

#endif /* EX_1_H */
