#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>

/// Info about project////
// 768 free RAM
// Actually about 1 GB of ram free

uint32_t fiveT = 536870912;
uint32_t twoF = 268435456;

// Function to compare ints in ascending order
int compare(const void* one, const void* two) {
    return (*(uint32_t*)one - *(uint32_t*)two);
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
    uint64_t size = st.st_size;

    // Open a file to the in and out files
    FILE* inFile = fopen(inFileName, "rb+");
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
        // Temporary size variable for while loop 
        uint64_t sizeCmp = size;

        // Decide how big chunks should be
        while (sizeCmp > 0) {
            // If there is more than 512 megabytes
            if (sizeCmp >= fiveT) {
                printf("in the if I thought\n");
                uint32_t numNums = twoF / 4;
                
                // First array
                uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr1, sizeof(uint32_t), numNums, inFile);
                
                strcat(fileName, "a");
                char* fName1 = (char*) malloc(sizeof(fileName));
                strcpy(fName1, fileName);
                
                qsort(arr1, numNums, sizeof(uint32_t), compare);
                FILE* tempFile = fopen(fName1, "wb+");
                fwrite(arr1, sizeof(uint32_t), numNums, tempFile);
                fclose(tempFile);
                
                // Freeing stuff
                free(arr1);
                free(fName1);
                
                // Second array
                uint32_t* arr2 = (uint32_t*) malloc(sizeof(uint32_t) * numNums);
                fread(arr2, sizeof(uint32_t), numNums, inFile);

                strcat(fileName, "a");
                char* fName2 = (char*) malloc(sizeof(fileName));
                strcpy(fName2, fileName);
                
                qsort(arr2, numNums, sizeof(uint32_t), compare);
                tempFile = fopen(fName2, "wb+");
                fwrite(arr2, sizeof(uint32_t), numNums, tempFile);
                fclose(tempFile);
                
                // Freeing stuff
                free(arr2);
                free(fName2);

                // Substract from temp size
                sizeCmp -= fiveT;
            }

            // Have to do math to figure out sizes of stuff
            else {
                // If the size left is between the 512 and 256 MB
                if (sizeCmp > twoF) {
                    printf("not the one i thought\n");

                    // First array
                    uint32_t arrsz = twoF / 4;
                    uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr1, sizeof(uint32_t), arrsz, inFile);
                    
                    strcat(fileName, "a");
                    char* fName1 = (char*) malloc(sizeof(fileName));
                    strcpy(fName1, fileName);

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

                    qsort(arr2, arrsz, sizeof(uint32_t), compare);
                    tempFile = fopen(fName2, "wb+");
                    fwrite(arr2, sizeof(uint32_t), arrsz, tempFile);
                    fclose(tempFile);
                
                    // Freeing stuff
                    free(arr2);
                    free(fName2);

                    // Set the size value to 0 since we are done
                    sizeCmp -= sizeCmp;
                }
                
                // Just create one more file
                // If the size is 256 MB or less
                else {
                    printf("in the last else\n");
                    
                    uint32_t arrsz = sizeCmp / 4;
                    uint32_t* arr1 = (uint32_t*) malloc(sizeof(uint32_t) * arrsz);
                    fread(arr1, sizeof(uint32_t), arrsz, inFile);
                    
                    strcat(fileName, "a");
                    char* fName1 = (char*) malloc(sizeof(fileName));
                    strcpy(fName1, fileName);
                    
                    qsort(arr1, arrsz, sizeof(uint32_t), compare);
                    FILE* tempFile = fopen(fName1, "wb+");
                    fwrite(arr1, sizeof(uint32_t), arrsz, tempFile);
                    fclose(tempFile);

                    // Subtract to zero since this is the last file
                    sizeCmp -= sizeCmp;

                    // Free stuff
                    free(arr1);
                    free(fName1);
                }
            }
        }

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
                if (pointers[i] > ((twoF/4)/strlen(fileName)) && bytesLeft[i] != 0 && buffsDone[i] != 1) {
                    printf("pointers[%d] :value = %d\n",i ,pointers[i]);
                    // Read in more data to that buffer
                    fread(arrayList[i], sizeof(uint32_t), ((twoF/4)/strlen(fileName)), fileList[i]);
                    pointers[i] = 0;
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
            if (outCounter == ((twoF / 4) / 2) || numBuffsDone == strlen(fileName)) {  
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

        // Freeing stuff and closing files and removing temp files
        char fName[32] = {};
        for (int i = 0; i < strlen(fileName); i ++) {
            strcat(fName, "a");
            free(arrayList[i]);
            fclose(fileList[i]);
            remove(fName);
        }
        free(outBuf);
        
    }

    // Close in and out files and return
    fclose(inFile);
    fclose(outFile);

    
    return 0;
}
