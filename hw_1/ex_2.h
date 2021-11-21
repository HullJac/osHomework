#ifndef EX_2_H
#define EX_2_H

#include <stdlib.h>
#include <string.h>

char* instr(char* str1, char* str2, int n) {
	// Get lengths of strings and total size needed
	// Keeping in mind the nullbyte at the end
	int str1Len = strlen(str1);
	int str2Len = strlen(str2);
	int totalSize = str1Len + str2Len + 1;
	// Make sure the insert place in actually within scope
	// Otherwise just add the second to the front
	if (n > 0 && n < (str1Len + 1)) {
		int loc = 0;
		// Create new string to return
		char* together = malloc(sizeof(char) * totalSize);
		
		// Put first part of the first string in new string
		for (int i = 0; i < n; i++) {
			together[loc] = str1[i];
			loc++;
		}
		// Put the second string where it needs to go
		for (int j = 0; j < str2Len; j++) {
			together[loc] = str2[j];
			loc++;
		}

		// Put first string second half after the second string
		int dif = str1Len - n;
		for (int k = n; k < str1Len; k++) {
			together[loc] = str1[k];
			loc++;
		}

		// Add null byte to the end
		together[loc] = '\0';
		return together;
	}
	// Add it to the front
	else {
		char* together = malloc(sizeof(char) * totalSize);
		strcpy(together, str2);
		strcat(together, str1);
		return together;
	}
}

#endif /* EX_2_H */
