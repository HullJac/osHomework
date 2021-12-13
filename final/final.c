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

//uint32_t fiveT = 536870912;
//uint32_t twoF = 268435456;
uint32_t fiveT = 512000000;
uint32_t twoF = 256000000;

struct chunkInfo {
    uint32_t* arrNums;
    uint32_t sz;
    char* name;

} typedef chunkInfo;


// Function to compare ints in ascending order
int compare(const void* one, const void* two) {
    return (*(uint32_t*)one - *(uint32_t*)two);
}

/*
// Threaded function to sort chunks of data
void* sortChunk(void* arg) {
    // Sort the data
    chunkInfo* ci = (chunkInfo*)arg;
    uint32_t* lst = ci -> arrNums;
    printf("This is the size we sould be sorting: %d\n", ci->sz),
    qsort(lst, ci->sz, sizeof(uint32_t), compare);

    // Write data to file
    FILE* tempFile = fopen(ci->name, "wb+");
    fwrite(lst, sizeof(uint32_t), ci->sz, tempFile);
    fclose(tempFile);

    return 0;
}
*/


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
    uint64_t size = st.st_size;

    // Open a file to the in and out files
    FILE* inFile = fopen(inFileName, "rb+");
    //printf("file pointer : %ld\n", sizeof(inFile));
    FILE* outFile = fopen(outFileName, "wb+");

    // If we can just sort everything in memory
    if (size < fiveT) { // Number here is 512 megabytes
        printf("in base if\n");
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
        //pthread_t tID1;
        //pthread_t tID2;

        uint64_t sizeCmp = size;
        // Decide how big chunks should be
        while (sizeCmp > 0) {
            // If I can launch 2 full threads
            if (sizeCmp >= fiveT) {
                printf("in the if I thought\n");
                uint32_t numNums = twoF / 4;
                
                // Create stucts
                chunkInfo strct1;
                chunkInfo strct2;

                // First array
                uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr1, sizeof(uint32_t), numNums, inFile);
                
                strcat(fileName, "a");
                char* fName1 = (char*) malloc(sizeof(fileName));
                strcpy(fName1, fileName);

                strct1.name = fName1;
                strct1.arrNums = arr1;
                strct1.sz = numNums;
                
                qsort(arr1, numNums, sizeof(uint32_t), compare);
                FILE* tempFile = fopen(fName1, "wb+");
                fwrite(arr1, sizeof(uint32_t), numNums, tempFile);
                fclose(tempFile);
                
                free(arr1);
                free(fName1);
                
                // Second array
                uint32_t* arr2 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr2, sizeof(uint32_t), numNums, inFile);

                strcat(fileName, "a");
                char* fName2 = (char*) malloc(sizeof(fileName));
                strcpy(fName2, fileName);
                
                strct2.name = fName2;
                strct2.arrNums = arr1;
                strct2.sz = numNums;
                
                qsort(arr2, numNums, sizeof(uint32_t), compare);
                tempFile = fopen(fName2, "wb+");
                fwrite(arr2, sizeof(uint32_t), numNums, tempFile);
                fclose(tempFile);

                sizeCmp -= fiveT;
                
                /*
                // Launch threads
                pthread_create(&tID1, NULL, sortChunk, &strct1);
                pthread_create(&tID2, NULL, sortChunk, &strct2);
                
                // Join threads
                pthread_join(tID1, NULL);
                pthread_join(tID2, NULL);
                */

                // Freeing stuff
                free(arr2);
                free(fName2);
            }

            // have to do math to figure out sizes of stuff
            //TODO Check case of only having one number here to sort
            //TODO check case of having an odd number of numbers
            else {
                if (sizeCmp > twoF) {
                    printf("not the one i thought\n");
                    // Create stucts
                    chunkInfo strct1;
                    chunkInfo strct2;

                    // First array
                    uint32_t arrsz = twoF / 4;
                    uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr1, sizeof(uint32_t), arrsz, inFile);
                    
                    strcat(fileName, "a");
                    char* fName1 = (char*) malloc(sizeof(fileName));
                    strcpy(fName1, fileName);

                    strct1.name = fName1;
                    strct1.arrNums = arr1;
                    strct1.sz = arrsz;

                    qsort(arr1, arrsz, sizeof(uint32_t), compare);
                    FILE* tempFile = fopen(fName1, "wb+");
                    fwrite(arr1, sizeof(uint32_t), arrsz, tempFile);
                    fclose(tempFile);
                    
                    free(arr1);
                    free(fName1);

                    // Subtract what was just written
                    sizeCmp -= twoF;
                    
                    // Second array
                    arrsz = sizeCmp / 4;
                    uint32_t* arr2 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr2, sizeof(uint32_t), arrsz, inFile);
                    strcat(fileName, "a");
                    char* fName2 = (char*) malloc(sizeof(fileName));
                    strcpy(fName2, fileName);
                    
                    strct2.name = fName2;
                    strct2.arrNums = arr1;
                    strct2.sz = arrsz;

                    qsort(arr2, arrsz, sizeof(uint32_t), compare);
                    tempFile = fopen(fName2, "wb+");
                    fwrite(arr2, sizeof(uint32_t), arrsz, tempFile);
                    fclose(tempFile);

                    // Set the size value to 0 since we are done
                    sizeCmp -= sizeCmp;

                    /*
                    // Launch threads
                    pthread_create(&tID1, NULL, sortChunk, &strct1);
                    pthread_create(&tID2, NULL, sortChunk, &strct2);

                    // Join threads
                    pthread_join(tID1, NULL);
                    pthread_join(tID2, NULL);
                    */

                    // Freeing stuff
                    free(arr2);
                    free(fName2);
                }
                
                // Just create one more file
                else {
                    printf("in the last else\n");
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
                    
                    qsort(arr1, arrsz, sizeof(uint32_t), compare);
                    FILE* tempFile = fopen(fName1, "wb+");
                    fwrite(arr1, sizeof(uint32_t), arrsz, tempFile);
                    fclose(tempFile);

                    // Subtract to zero since this is the last file
                    sizeCmp -= sizeCmp;

                    /*
                    pthread_create(&tID1, NULL, sortChunk, &strct1);
                    pthread_join(tID1, NULL);
                    */

                    // Free stuff
                    free(arr1);
                    free(fName1);
                }
            }
        }

        // Bools for main loop
        uint8_t empty = 0;

        // Keeps track of number of completed files
        uint8_t buffsDone[strlen(fileName)];
        uint8_t numBuffsDone = 0;

        // List of file handles
        char fn[32] = {};
        FILE* fileList[strlen(fileName)];

        // Create out buffer 128 megabytes
        uint32_t outCounter = 0;
        uint32_t* outBuf = (uint32_t*) malloc(sizeof(uint32_t) * ((twoF / 4) / 2));
        
        // Create list of pointers into buffers
        uint32_t pointers[strlen(fileName)];

        // List to keep track of filesizes left
        uint64_t bytesLeft[strlen(fileName)];
        
        // Array of pointers to hold file buffers
        uint32_t* arrayList[strlen(fileName)];

        for (uint32_t i = 0; i < strlen(fileName); i++) {
            // Open all the files and add the pointers to a list
            strcat(fn, "a");
            fileList[i] = fopen(fn, "rb+");
            
            // Malloc all the buffers
            arrayList[i] = (uint32_t*) malloc(sizeof(uint32_t) * ((twoF / 4) / strlen(fileName)));
            
            // Read data into the buffers
            uint32_t r = fread(arrayList[i], sizeof(uint32_t), ((twoF / 4) / strlen(fileName)), fileList[i]);
            
            //TODO check if r == 0
            // Fill the pointer list that reference the buffer offsets
            pointers[i] = 0;

            // Fill buffs done array
            buffsDone[i] = 0;

            // Fill the bytesLeft array
            struct stat s;
            stat(fn, &st);
            bytesLeft[i] = st.st_size;
        }

        // Compare stuff in buffers until one is empty 
        // Also check if the out buffer is full
        uint32_t smallestNum = arrayList[0][0];
        uint32_t smallestPos = 0;

        while (numBuffsDone != strlen(fileName)) {
            // Do comparison to find lowest number of the buffers
            for (int i = 0; i < strlen(fileName); i++) {
                // Check if any of the pointers are at the end of the buffer size
                // TODO check for weird case of extra chunk of smaller size
                if (pointers[i] > ((twoF/4)/strlen(fileName)) && bytesLeft[i] != 0 && buffsDone[i] != 1) {
                    printf("pointers[%d] :value = %d\n",i ,pointers[i]);
                    //printf("file address[%d] = %p\n", i, fileList[i]);
                    // Read in more data to that buffer
                    uint32_t bytesRead = fread(arrayList[i], sizeof(uint32_t), ((twoF/4)/strlen(fileName)), fileList[i]);
                    //printf("bytesRead at [%d] = %d\n",i, bytesRead);
                    pointers[i] = 0;
                    //printf("%d : %d\n",i, pointers[i]);
                    //printf("bytesread %d: %d\n",i ,bytesRead);
                    //fflush(stdout);
                }

                if (bytesLeft[i] == 0);
                
                else if (smallestNum > arrayList[i][pointers[i]]) {
                    smallestNum = arrayList[i][pointers[i]];
                    smallestPos = i;
                }
            }
                    
            // Increment the offset of the buffer taken from
            pointers[smallestPos] ++;

            // Decrement the number of bytes left in the file taken from 
            bytesLeft[smallestPos] -= 4;
            if (bytesLeft[smallestPos] == 0) {
                buffsDone[smallestPos] = 1;
                numBuffsDone ++;
            }

            // Grab the lowest and put it in outBuf
            outBuf[outCounter] = smallestNum;

            // Increment outCounter
            outCounter ++;

            // If the outBuffer is full
            if (outCounter == ((twoF / 4) / 2)) {  
                //printf("outcounter %d : %d\n", outCounter, ((twoF / 2) / 4));
                //printf("filesDone %d : %ld\n", filesDone, strlen(fileName));
                fwrite(outBuf, sizeof(uint32_t), outCounter, outFile);
                outCounter = 0;
            }
            
            // Reset the smallest variables 
            for (int i = 0; i < strlen(fileName); i++) {
                if (buffsDone[i] != 1) {
                    smallestNum = arrayList[i][pointers[i]]; 
                    smallestPos = i;
                    break;
                }
            }
        }

        // Freeing stuff
        for (int i = 0; i < strlen(fileName); i ++) {
            free(arrayList[i]);
            fclose(fileList[i]);
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
