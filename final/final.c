#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/stat.h>

// Function to compare ints in ascending order
int compare(const void* one, const void* two) {
    return (*(int*)one - *(int*)two);
}


// Threaded function to sort chunks of data
void* sortChunk(void* arg) {
    qsort(arg, sizeof(arg)/sizeof(unsigned int), sizeof(unsigned int), compare);
}



int main(int argc, char* argv[]) {
    char* inFile;
    char* outFile;

    // Handles command line input
    if (argc == 3) {
        inFile = argv[1];
        outFile = argv[2];
    }
    else if (argc > 3) {
        printf("Too many arguments given.\n");
    }
    else if (argc == 2) {
        printf("You need to add one more argument.\n");
        printf("You only specififed the input or output file.\n");
    }
    else if (argc == 1) {
        printf("You need to specify an input and output file.\n");
    }
    else {
        fprintf(stderr,"Something when wrong with input.\n");
    }

    // Num threads = 4
    
    // Get how big a file is
    struct stat st;
    stat(inFile, &st);
    uint32_t size = st.st_size;

    printf("%d\n", size);

    // Decide how big chunks should be
    uint32_t numNums = size/sizeof(size);
    printf("%d\n", numNums);

    // Grab chunks and sort them and put them in files
    
    // Grab buffers from each file
    
    // When a buffer is empty, grab more until it is all gone, then erase the file

    
    return 0;
}
