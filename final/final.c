#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>

/// Info about project////
// Num threads = 4
// 768 free RAM

struct chunkInfo {
    uint32_t* arrNums;
    uint32_t sz;
    char* name;

} typedef chunkInfo;


// Function to compare ints in ascending order
int compare(const void* one, const void* two) {
    return (*(int*)one - *(int*)two);
}


// Threaded function to sort chunks of data
void* sortChunk(void* arg) {
    // Sort the data
    chunkInfo* ci = (chunkInfo*)arg;
    qsort(ci->arrNums, ci->sz, sizeof(uint32_t), compare);
        
    // Write data to file
    FILE* tempFile = fopen(ci->name, "wb+");
    fwrite(ci->arrNums, sizeof(uint32_t), ci->sz, tempFile);
    fclose(tempFile);

    return NULL;
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

    // File information
    char fileName[64] = {};
    
    // Get how big a file is in bytes and how many numbers there are
    struct stat st;
    stat(inFileName, &st);
    uint32_t size = st.st_size;

    // Open a file to the in and out files
    FILE* inFile = fopen(inFileName, "rb+");
    printf("file pointer : %ld\n", sizeof(inFile));
    FILE* outFile = fopen(outFileName, "wb+");

    // If we can just sort everything in memory
    if (size < 536870912) { // Number here is 512 megabytes
        uint32_t numNums = size/4;
        
        // Create and array the size of the number of ints we have
        uint32_t* arr = (uint32_t*)malloc(sizeof(uint32_t) * numNums);

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
        // Create 2 threads
        pthread_t tID1;
        pthread_t tID2;

        uint32_t sizeCmp = size;
        // Decide how big chunks should be
        while (sizeCmp > 0) {
            // If I can launch 2 full threads
            if (sizeCmp >= 536870912) {
                uint32_t numNums = 268435456 / 4;
                
                // Create stucts
                chunkInfo strct1;
                chunkInfo strct2;

                // Add stuff to structs
                uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr1, sizeof(uint32_t), numNums, inFile);
                
                strcat(fileName, "a");
                char* fName1 = (char*) malloc(sizeof(fileName));
                strcpy(fName1, fileName);

                strct1.name = fName1;
                strct1.arrNums = arr1;
                strct1.sz = numNums;
                
                uint32_t* arr2 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr2, sizeof(uint32_t), numNums, inFile);
                strcat(fileName, "a");
                char* fName2 = (char*) malloc(sizeof(fileName));
                strcpy(fName2, fileName);
                
                strct2.name = fName2;
                strct2.arrNums = arr1;
                strct2.sz = numNums;

                sizeCmp -= 536870912;
                
                // Launch threads
                pthread_create(&tID2, NULL, sortChunk, &strct2);
                pthread_create(&tID1, NULL, sortChunk, &strct1);
                
                // Join threads
                pthread_join(tID1, NULL);
                pthread_join(tID2, NULL);
            
                // Freeing stuff
                free(arr1);
                free(arr2);
                free(fName1);
                free(fName2);
            }

            // have to do math to figure out sizes of stuff
            //TODO Check case of only having one number here to sort
            else {
                // Divide bytes left in half
                uint32_t numNums = (sizeCmp / 4) / 2;

                // Create stucts
                chunkInfo strct1;
                chunkInfo strct2;

                // Add stuff to structs
                uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr1, sizeof(uint32_t), numNums, inFile);
                
                strcat(fileName, "a");
                char* fName1 = (char*) malloc(sizeof(fileName));
                strcpy(fName1, fileName);

                strct1.name = fName1;
                strct1.arrNums = arr1;
                strct1.sz = numNums;
                
                uint32_t* arr2 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr2, sizeof(uint32_t), numNums, inFile);
                strcat(fileName, "a");
                char* fName2 = (char*) malloc(sizeof(fileName));
                strcpy(fName2, fileName);
                
                strct2.name = fName2;
                strct2.arrNums = arr1;
                strct2.sz = numNums;

                sizeCmp -= (numNums * 2);
                
                // Launch threads
                pthread_create(&tID2, NULL, sortChunk, &strct2);
                pthread_create(&tID1, NULL, sortChunk, &strct1);
                
                // Join threads
                pthread_join(tID1, NULL);
                pthread_join(tID2, NULL);
            
                // Freeing stuff
                free(arr1);
                free(arr2);
                free(fName1);
                free(fName2);
            }
        }
        
        // List of file handle
        char fn[64] = {};
        //FILE** fileList = (FILE**) malloc(sizeof(FILE*) * strlen(fileName));
        printf("%ld\n", strlen(fileName));
        FILE* fileList[strlen(fileName)];
        for (int i = 0; i < strlen(fileName); i++) {
            strcat(fn, "a");
            //FILE* f = fopen(fn, "rb+");
            fileList[i] = fopen(fn, "rb+");
            //fileList[i] = f;
            //memcpy(fileList[i], f, sizeof(f));
        }
        
        // Create out buffer 
        uint32_t* outBuf = (uint32_t*) malloc(sizeof(uint32_t) * ((268435456 / 2) / 4));
        
        // Array of arrays to hold file buffers
        uint32_t* arrayList = (uint32_t*) malloc(sizeof(uint32_t) * (strlen(fileName) * (268435456 / 4) / strlen(fileName)));


        // Grab buffers from each file
        

        // When a buffer is empty, grab more until it is all gone, then erase the file
        

        // To erase a file use remove("filename")

        // Freeing stuff
        //free(fileList);  ///////////TODO
        free(arrayList);
        free(outBuf);
    }


    // Close files and return
    fclose(inFile);
    fclose(outFile);
    
    return 0;
}


// 1024*1024*256 = 256 megabytes
// /sizof(uint32_t) = number of ints in 256 megabytes
        
                //printf("%s : %s\n", strct1.name, strct2.name);

//printf("here\n");
        //fflush(stdout);
                /*
                for (int i = 0; i< 1000; i++) {
                    printf("%d\n", arr1[i]);
                }
                */





