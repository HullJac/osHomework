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
    char fileName[32] = {};
    
    // Get how big a file is in bytes and how many numbers there are
    struct stat st;
    stat(inFileName, &st);
    uint32_t size = st.st_size;

    // Open a file to the in and out files
    FILE* inFile = fopen(inFileName, "rb+");
    //printf("file pointer : %ld\n", sizeof(inFile));
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
            //TODO check case of having an odd number of numbers
            else {
                if (sizeCmp > 268435456) {
                    // Create stucts
                    chunkInfo strct1;
                    chunkInfo strct2;

                    // Add stuff to structs
                    uint32_t arrsz = 268435456 / 4;
                    uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr1, sizeof(uint32_t), arrsz, inFile);
                    
                    strcat(fileName, "a");
                    char* fName1 = (char*) malloc(sizeof(fileName));
                    strcpy(fName1, fileName);

                    strct1.name = fName1;
                    strct1.arrNums = arr1;
                    strct1.sz = arrsz;

                    // Subtract what was just written
                    sizeCmp -= 268435456;
                    
                    arrsz = sizeCmp / 4;
                    uint32_t* arr2 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr2, sizeof(uint32_t), arrsz, inFile);
                    strcat(fileName, "a");
                    char* fName2 = (char*) malloc(sizeof(fileName));
                    strcpy(fName2, fileName);
                    
                    strct2.name = fName2;
                    strct2.arrNums = arr1;
                    strct2.sz = arrsz;

                    // Set the size value to 0 since we are done
                    sizeCmp -= sizeCmp;
                    
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
                
                // Just create one more file
                else {
                    chunkInfo strct1;
                    
                    uint32_t arrsz = sizeCmp / 4;
                    uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr1, sizeof(uint32_t), arrsz, inFile);
                    
                    strcat(fileName, "a");
                    char* fName1 = (char*) malloc(sizeof(fileName));
                    strcpy(fName1, fileName);

                    strct1.name = fName1;
                    strct1.arrNums = arr1;
                    strct1.sz = arrsz;

                    // Subtract to zero since this is the last file
                    sizeCmp -= sizeCmp;

                    pthread_create(&tID1, NULL, sortChunk, &strct1);
                    pthread_join(tID1, NULL);
                    
                    // Free stuff
                    free(arr1);
                    free(fName1);
                }
            }
        }

        // Bools for main loop
        uint8_t empty = 0;

        // Keeps track of number of completed files
        uint8_t filesDone = 0;

        // List of file handles
        char fn[32] = {};
        FILE* fileList[strlen(fileName)];

        // Create out buffer 128 megabytes
        uint32_t outCounter = 0;
        uint32_t* outBuf = (uint32_t*) malloc(sizeof(uint32_t) * ((268435456 / 2) / 4));
        
        // Create list of pointers into buffers
        long long int pointers[strlen(fileName)];
        
        // Array of pointers to hold file buffers
        uint32_t* arrayList[strlen(fileName)];

        for (uint32_t i = 0; i < strlen(fileName); i++) {
            // Open all the files and add the pointers to a list
            strcat(fn, "a");
            fileList[i] = fopen(fn, "rb+");
            
            // Malloc all the buffers
            arrayList[i] = (uint32_t*) malloc(sizeof(uint32_t) * ((268435456 / 4) / strlen(fileName)));
            
            // Read data into the buffers
            uint32_t r = fread(arrayList[i], sizeof(uint32_t), ((268435456 / 4) / strlen(fileName)), fileList[i]);
            
            // Fill the pointer list that reference the buffer offsets
            pointers[i] = 0;
        }

        // Compare stuff in buffers until one is empty 
        // Also check if the out buffer is full
        uint32_t smallestNum = arrayList[0][0];
        uint32_t smallestPos = 0;
        while (empty != 1) {
            // Do comparison to find lowest number of the buffers
            for (int i = 0; i < strlen(fileName); i++) {
                // Check if any of the pointers are at the end of the buffer size
                // TODO check for weird case of extra chunk of smaller size
                //printf("%d\n", pointers[i]);
                if (pointers[i] > ((268435456 / 4) / strlen(fileName))) {
                    printf("pointers at i %d : %lld\n",i ,pointers[i]);
                    // Read in more data to that buffer
                    uint32_t bytesRead = fread(arrayList[i], sizeof(uint32_t), ((268435456 / 4) / strlen(fileName)), fileList[i]);
                    printf("reading : %d\n", i);
                    printf("bytesRead : %d\n", bytesRead);
                    pointers[i] = 0;
                    
                    //printf("bytesread %d: %d\n",i ,bytesRead);
                    //fflush(stdout);
                    if (bytesRead == 0) {
                        pointers[i] = -1;
                        filesDone ++;
                    }
                }

                if (pointers[i] == -1);
                
                else if (smallestNum > arrayList[i][pointers[i]]) {
                    smallestNum = arrayList[i][pointers[i]];
                    smallestPos = i;
                }
            }
                    
            // Increment the offset of the buffer taken from
            pointers[smallestPos] ++;

            // Grab the lowest and put it in outBuf
            outBuf[outCounter] = smallestNum;

            // Increment outBuf
            outCounter ++;

            // If the outBuffer is full
            // TODO check if write moves the pointer
            if (outCounter == ((268435456 / 2) / 4) || filesDone == strlen(fileName)) {
                //printf("outcounter %d : %d\n", outCounter, ((268435456 / 2) / 4));
                //printf("filesDone %d : %ld\n", filesDone, strlen(fileName));
                fwrite(outBuf, sizeof(uint32_t), outCounter, outFile);
                outCounter = 0;
            }
            
            // Check if all the buffers are empty
            if (filesDone == strlen(fileName)) {
                //printf("in filesDone\n");
                //fflush(stdout);
                empty = 1;
            }
            
            // Reset the smallest variables 
            for (int i = 0; i < strlen(fileName); i++) {
                if (pointers[i] != -1) {
                    smallestNum = arrayList[i][pointers[i]]; 
                    smallestPos = i ;
                    break;
                }
            }
        }

        // Freeing stuff
        for (int i = 0; i < strlen(fileName); i ++) {
            free(arrayList[i]);
        }
        free(outBuf);

    }

    // Close files and return
    fclose(inFile);
    fclose(outFile);

    // TODO Remove all the extra files
    // To erase a file use remove("filename")
    
    return 0;
}

// FUCK YA LIFE BING BONG
// 1024*1024*256 = 256 megabytes
// /sizof(uint32_t) = number of ints in 256 megabytes
