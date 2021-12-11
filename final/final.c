#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>

/// Info about project////
// Num threads = 4
// 768 free RAM

// Function to compare ints in ascending order
int compare(const void* one, const void* two) {
    return (*(int*)one - *(int*)two);
}


// Threaded function to sort chunks of data
void* sortChunk(void* arg) {
    qsort(arg, sizeof(arg)/sizeof(unsigned int), sizeof(unsigned int), compare);
}



int main(int argc, char* argv[]) {
    char* inFileName;
    char* outFileName;

    // Handles command line input
    if (argc == 3) {
        inFileName = argv[1];
        outFileName = argv[2];
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

    
    // Get how big a file is in bytes
    struct stat st;
    stat(inFileName, &st);
    uint32_t size = st.st_size;
    //printf("%d\n", size);

    // Open a file to the in and out files
    FILE* inFile = fopen(inFileName, "rb+");
        //(inFile, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);
    FILE* outFile = fopen(outFileName, "wb+");
        //(outFile, O_CREAT | O_RDWR | O_EXCL, 0644);

    // If we can just sort everything in memory
    if (size <= 536870912) { // Number here is 512 megabytes
        // Create and array the size of the number of ints we have
        uint32_t numNums = size/4;
        int* arr = (uint32_t*)malloc(sizeof(uint32_t) * numNums);

        // Read in the ints from the file
        fread(arr, sizeof(uint32_t), numNums, inFile);

        // Sort them using qsort
        qsort(arr, numNums, sizeof(uint32_t), compare);
        
        // Write the array to the file
        fwrite(arr, sizeof(uint32_t), numNums, outFile);

        // Free the malloced array
        free(arr);
    }

    // If the data should be split up into files
    else {
        // Decide how big chunks should be
        uint32_t numNums = size/4;
        printf("%d\n", numNums);

        // Grab chunks and sort them and put them in files
        
        // Grab buffers from each file
        
        // When a buffer is empty, grab more until it is all gone, then erase the file
    }
    
    return 0;
}


// 1024*1024*256 = 256 megabytes
// /sizof(uint32_t) = number of ints in 256 megabytes
        

//printf("here\n");
        //fflush(stdout);
