/* CMSC 432 - Homework 6
 * Assignment Name: bvfs - the BV File System
 * Due: Tuesday, November 30th @ 11:59 p.m.
 */


/*
 * [Requirements / Limitations]
 *   Partition/Block info
 *     - Block Size: 512 bytes
 *     - Partition Size: 8,388,608 bytes (16,384 blocks)
 *
 *   Directory Structure:
 *     - All files exist in a single root directory
 *     - No subdirectories -- just a list of file names
 *
 *   File Limitations
 *     - File Size: Maximum of 65,536 bytes (128 blocks)
 *     - File Names: Maximum of 32 characters including the null-byte
 *     - 256 file maximum -- Do not support more
 *
 *   Additional Notes
 *     - Create the partition file (on disk) when bvfs_init is called if the file
 *       doesn't already exist.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

//TODO
//TODO Run this on Kay so that I make sure the blocks are all still 512 and stuff lines up and works
//TODO
//
//
//TODO Look into why when I run the file the second time, the Inodes get a lot bigger. 

// Struct to represent the Inodes in this file system
struct Inode {
    // I decided to make these all 32 bit because it didn't really matter and helped me 
    // not have to change the padding when I compiled using the make file or my own stuff
    char name[32];          // Name of the file
    uint32_t fd;             // ID of the file is one more than the nodes place in the array
    uint32_t numBlocks;      // Number of blocks in the file
    uint32_t opened;         // Int to determine how file is open 
    // 0 = closed, 1 = read, 2 = append, 3 = truncate
    uint32_t lastDB;         // Last dataBlock that has free space in dataBlockAddresses
    uint32_t nextFreeByte;  // Next free byte in the last dataBlock
    uint32_t numBytes;      // Bytes in the file
    uint16_t dataBlockAddresses[128]; // Diskmap represented as index of the block
    // Meaning I need to multiply by blockSize to get actual location
    time_t lastModTime;     // Last time anything happened to the file
    char padding[196]; // Extra padding to fill the block size

} typedef Inode;


// Struct to represent the one super block in this file system
struct superBlock {
    uint16_t remainingFiles;    // Number of files left
    uint16_t firstFreeList;     // Represented as an index of blocks
    uint32_t remainingBlocks;   // Number of blocks left
    char padding[504];          // Extra padding to fill the block size

} typedef superBlock;


// Struct to represent the empty space
struct freeSpace {
    uint16_t freeBlocks[255];
    uint16_t nextFreeSpaceBlock;

} typedef freeSpace;



//************ Global Variables ************//
uint8_t inited = 0;             // Bool to check if init has been called
uint16_t blockSize = 512;       // Reference to size of block in our file system
uint16_t maxFreeBlocks = 63;    // Most Free node we can have hard coded sadly
uint16_t maxBlocks = 16384;     // Max number of block available in file system
uint16_t maxFiles = 256;        // Max number of files that can be stored in file system
int maxFileBytes = 65536;       // Max number of bytes per file
superBlock SB;                  // Super block to manipulate later
freeSpace FS;                   // First free space block 
Inode INList[256];              // Inode list to store all inodes
freeSpace emptyBlock = {0, 0};  // Empty struct to use when reclaiming space 
int pFD;                        // Make the file descriptor global
//************ Global Variables ************//


// Prototypes
int bvfs_init(const char *fs_fileName);
int bvfs_detach();
int bvfs_open(const char *fileName, int mode);
int bvfs_close(int bvfs_FD);
int bvfs_write(int bvfs_FD, const void *buf, size_t count);
int bvfs_read(int bvfs_FD, void *buf, size_t count);
int bvfs_unlink(const char* fileName);
void bvfs_ls();


///////////////////
//Helper functions/
///////////////////

// Finds first free block
// Sets that address to taken
// Returns a pointer to that block
// If needed, will move to the next free block and return the address
// of the empty free block while changing the head pointer of the free block list
uint16_t getFreeBlock() { //TODO check places this is called to make sure it doesn't get -1
                          //TODO if it does, make sure to check for it
    // First check to see if there are any free blocks available
    if (SB.remainingBlocks == 0) {
        //fprintf(stderr, "There are no more free block to give.");
        return -1;
    }

    // Find the first free block
    uint16_t block;
    for (uint16_t i = 0; i < 255; i++) {
        block = FS.freeBlocks[i];
        if (block != 0) {
            // Set the location in the freeblock list to 0
            FS.freeBlocks[i] = 0;
            
            // Update the super block count
            SB.remainingBlocks = SB.remainingBlocks - 1;
            lseek(pFD, 0, SEEK_SET);
            write(pFD, (void*)&SB, sizeof(SB));

            // Write the free block back to disk
            lseek(pFD, SB.firstFreeList*blockSize, SEEK_SET);
            write(pFD, (void*)&FS, sizeof(FS));
            
            // Return the pointer to that block given
            return block;
        }
    }
    // Otherwise, if there are no free spaces left in the current block
    // move to the next free node block
    if (block == 0) {
        // Store the address of the current first free block to return later
        // This will be the block that we give them to use
        block = SB.firstFreeList; 
        
        // Reset superBlock pointer
        SB.firstFreeList = FS.nextFreeSpaceBlock;
        
        // Seek to the next freeSpace block
        lseek(pFD, FS.nextFreeSpaceBlock*blockSize, SEEK_SET);

        // Read the next freeBlock and put it in FS
        read(pFD, (void*)&FS, sizeof(FS));
       
        // Write the SB back to the file
        lseek(pFD, 0, SEEK_SET);
        write(pFD, (void*)&SB, sizeof(SB));

        // Return the block
        return block;
    }
}


// Adds a block back to the free space
// Finds the first open block and puts the corresponding "pointer" there
// If all blocks are full, it will create an new empty on and make it the head node
void giveBackBlock(int blk) {
    // Try to find a spot in the current first free block
    int block;
    int ifStatement = 0;
    for (int i = 0; i < 255; i++) {
        block = FS.freeBlocks[i];
        if (block == 0) {
            ifStatement = 1;
            // Put the pointer in the freeblock
            FS.freeBlocks[i] = blk;

            // Update the super block count
            SB.remainingBlocks = SB.remainingBlocks + 1;
            lseek(pFD, 0, SEEK_SET);
            write(pFD, (void*)&SB, sizeof(SB));
            
            // Write the freeblock to disk
            lseek(pFD, SB.firstFreeList*blockSize, SEEK_SET);
            write(pFD, (void*)&FS ,sizeof(FS)); 

            // Write over the block given back
            lseek(pFD, blk*blockSize, SEEK_SET);
            write(pFD, (void*)&emptyBlock, sizeof(emptyBlock));

            // Break so the function stops
            break;
        }
    }
    
    //TODO think about if these blocks need to count as free blocks too
    //
    // Else create a new free block node and put it in the list
    if (ifStatement == 0) {
        // List of free spaces to put in the new node which is empty
        uint16_t freeList[255];

        // Fill the free list with zeros to put in the new node
        for (uint16_t i = 0; i < 255; i++) {
            freeList[i] = 0;
        }
        
        // Create node and add stuff to it
        freeSpace newFree;
        memcpy(newFree.freeBlocks, freeList, sizeof(freeList));
        newFree.nextFreeSpaceBlock = SB.firstFreeList;
        
        // Write the node to the file at the block given back
        // So, no need to overwrite the block given back this will
        lseek(pFD, blk*blockSize, SEEK_SET);
        write(pFD, (void*)&newFree, sizeof(newFree));

        // Rearange the head pointer to point to the new block
        SB.firstFreeList = blk;
        
        // Write the SB back to the file
        lseek(pFD, 0, SEEK_SET);
        write(pFD, (void*)&SB, sizeof(SB));
    }
}


// Finds the corresponding file descriptor given a file name
int findFileDesc(const char *fileName) {
    // Search through the Inodes to see if any of them have the name passed to this funciton
    for (int i = 0; i < maxFiles; i++) {
        if (strcmp(INList[i].name, fileName) == 0) {
            return INList[i].fd;
        }
    }

    // Otherwise return -1 to signify not found
    //fprintf(stderr, "Filename: <%s> not found\n", fileName);
    return -1;
}


// Finds the first open Inode
// Returns the index into the Inode array
int findFreeFile() {
    // Search through the Inodes to see if any of them are free
    for (int i = 0; i < maxFiles; i++) {
        if (INList[i].lastModTime == 0) {
            return i;
        }
    }

    // Otherwise return -1 to signify all files are taken
    return -1;
}


// Closes all files
void closeAllFiles() {
    // Change all files to closed and update the time
    for (int i = 0; i < maxFiles; i++) {
        if (INList[i].lastModTime != 0) {
            INList[i].lastModTime = time(NULL);
            INList[i].opened = 0;
        }
    }
    
    // Write Inode array back to disk
    lseek(pFD, blockSize, SEEK_SET);
    write(pFD, (void*)&INList, sizeof(INList));
}


// Finds the index of all the files present
int* findAllFiles() {
    // Create a static array to reference
    static int files[256];

    // Index to add stuff to the files array at
    int fIndex = 0;

    // Search through all the Inodes to see if any have files in them
    for (int i = 0; i < maxFiles; i++) {
        if (INList[i].lastModTime != 0) {
            files[fIndex] = i;
            fIndex = fIndex + 1;
        }
    }

    // Return the pointer to the array
    return files;
}






/*
 * int bvfs_init(const char *fs_fileName);
 *
 * Initializes the bvfs file system based on the provided file. This file will
 * contain the entire stored file system. Invocation of this function will do
 * one of two things:
 *
 *   1) If the file (fs_fileName) exists, the function will initialize in-memory
 *   data structures to help manage the file system methods that may be invoked.
 *
 *   2) If the file (fs_fileName) does not exist, the function will create that
 *   file as the representation of a new file system and initialize in-memory
 *   data structures to help manage the file system methods that may be invoked.
 *
 * Input Parameters
 *   fs_fileName: A c-string representing the file on disk that stores the bvfs
 *   file system data.
 *
 * Return Value
 *   int:  0 if the initialization succeeded.
 *        -1 if the initialization failed (eg. file not found, access denied,
 *           etc.). Also, print a meaningful error to stderr prior to returning.
 */
int bvfs_init(const char *fs_fileName) {
    pFD = open(fs_fileName, O_CREAT | O_RDWR | O_EXCL, 0644); // Last parameter is permissions
    if (pFD < 0) {
        if (errno == EEXIST) { // File already exists. 
            // Open it and read data for in-memory data structures 
            pFD = open(fs_fileName, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);

            // Read the super block
            read(pFD, (void*)&SB, sizeof(SB));
            //printf("Read from file: %d + %d + %d + [%s]\n", SB.remainingFiles, SB.firstFreeList, SB.remainingBlocks SB.padding);
            
            // Grab the Inode list and store it in memory
            lseek(pFD, blockSize, SEEK_SET);
            read(pFD, (void*)&INList ,sizeof(INList));

            /*
            printf("First Inode name: <%s>\n", INList[0].name);
            printf("First Inode fd: <%d>\n", INList[0].fd);
            printf("First Inode numBlocks: <%d>\n", INList[0].numBlocks);
            printf("First Inode opened: <%d>\n", INList[0].opened);
            printf("First Inode lastBD: <%d>\n", INList[0].lastBD);
            printf("First Inode nextFreeByte: <%d>\n", INList[0].nextFreeByte);
            printf("First Inode numBytes: <%d>\n", INList[0].numBytes);
            printf("First Inode time: <%ld>\n", INList[0].lastModTime);
            printf("First Inode dataBlockAddresses[0] <%d>\n", INList[0].dataBlockAddresses[0]);
            */

            // Seek to where the first free block is
            lseek(pFD, SB.firstFreeList*blockSize, SEEK_SET);

            // Read the first freeBlock based on the super block
            read(pFD, (void*)&FS, sizeof(FS));
            //printf("Read from free block: %d + %d\n", FS.freeBlocks[0], FS.nextFreeSpaceBlock);
            
            // Set inited to true
            inited = 1;

            return 0;

        }
        else {
            // Something bad must have happened... check errno
            // Then return -1 as stated above
            fprintf(stderr, "Error opening file: %s\n", strerror(errno));
            return -1;

        }
    }
    else {
        // File did not previously exist but it does now. Write needed meta data to it
        
        // Create and write the inital super block
        uint32_t numRemBlocks = 16384 - 1 - maxFiles - maxFreeBlocks;
        superBlock super = {256, 257, numRemBlocks, 0};
        //printf("super block: %ld\n", sizeof(super));
        write(pFD, (void*)&super, sizeof(super));

        // Create an array of Inodes
        Inode Ilst [256];
        
        // Create an empty Inode
        Inode IN = {0,0,0,0,0,0,0,0,0,0};
        //printf("inode: %ld\n", sizeof(IN));
        
        // Write empty Inode blocks to array
        for (int i = 0; i < 256; i++) {
            Ilst[i] = IN;
        }

        // Write Inode array to partition
        write(pFD, (void*)&Ilst, sizeof(Ilst));

        // Create and fill array to store in freeSpace struct 
        uint16_t freeSpot = 258;
        uint16_t freeList[255];
        uint16_t nextFreeNode;
        uint16_t freeListLoc = 0;

        for (uint16_t j = 0; j < maxFreeBlocks; j++) { // 63 iterations
            // Need to seek to next block each time
            lseek(pFD, (freeSpot-1)*blockSize, SEEK_SET);

            for (uint16_t i = freeSpot; i < (freeSpot + 256); i++) { // Fills a whole freeSpace block
                if (i >= maxBlocks) { // Don't add the next spot, add NULL to end and fill with zeros
                    nextFreeNode = 0; 
                    freeList[freeListLoc] = 0;
                    freeListLoc++;
                }
                else { // Add pointer to next free spot and recalculate the pointer to next free node
                    nextFreeNode = i;
                    freeList[freeListLoc] = i;
                    freeListLoc++;
                }
            }
            freeSpot += 256; // Increment the place where the next free spot starts
            freeListLoc = 0; // Reset the location inside the freeList

            // Create the freeSpace struct
            freeSpace FB; 
            memcpy(FB.freeBlocks, freeList, sizeof(freeList));
            FB.nextFreeSpaceBlock = nextFreeNode;
            //printf("free block: %ld\n", sizeof(FB));
            
            // Write the free block to file at the seeked location from above
            write(pFD, (void*)&FB, sizeof(FB));
        }

        // Make sure all free spaces after the last free node are accounted for
        // I fixed this issue by seeking to the end of the file and writing a zero
        char extra[1] = {0};
        lseek(pFD, 8388607, SEEK_SET);
        write(pFD, (void*)&extra, sizeof(extra));

        // Assign all the needed global variables
        // These should all be in order on a new init
        // So, only need to seek to the front of the file
        lseek(pFD, 0, SEEK_SET);
        read(pFD, (void*)&SB, sizeof(SB));
        read(pFD, (void*)&INList, sizeof(INList));
        read(pFD, (void*)&FS, sizeof(FS));

        // Let the user know that the partition was created
        //printf("Created Partition: %s\n", fs_fileName);

        // Set inited to true
        inited = 1;

        return 0;
    }
}






/*
 * int bvfs_detach();
 *
 * This is your opportunity to free any dynamically allocated resources and
 * perhaps to write any remaining changes to disk that are necessary to finalize
 * the bvfs file before exiting.
 *
 * Return Value
 *   int:  0 if the clean-up process succeeded.
 *        -1 if the clean-up process failed (eg. bvfs_init was not previously,
 *           called etc.). Also, print a meaningful error to stderr prior to
 *           returning.
 */
int bvfs_detach() { 
    //TODO check for more stuff to do here like writing FB and SB
    //TODO right now, I am handling writing everything when something changes
    //TODO check for anything that was malloced

    if (inited == 1) {
        // Close all the files
        closeAllFiles();
        // Set inited to 0 so nothing works
        inited = 0;
        return 0;
    }

    // Error that init was never called
    else {
        fprintf(stderr, "bvfs_init was never called.\n");
        return -1;
    }
}







// Available Modes for bvfs (see bvfs_open below)
int BVFS_RDONLY = 0;
int BVFS_WAPPEND = 1;
int BVFS_WTRUNC = 2;

/*
 * int bvfs_open(const char *fileName, int mode);
 *
 * This function is intended to open a file in either read or write mode. The
 * above modes identify the method of access to utilize. If the file does not
 * exist, you will create it, unless its read-only more. 
 * The function should return a bvfs file descriptor
 * for the opened file which may be later used with bvfs_(close/write/read).
 *
 * Input Parameters
 *   fileName: A c-string representing the name of the file you wish to fetch
 *             (or create) in the bvfs file system.
 *   mode: The access mode to use for accessing the file
 *           - BVFS_RDONLY: Read only mode
 *           - BVFS_WAPPEND: Write only mode, appending to the end of the file
 *           - BVFS_WTRUNC: Write only mode, replacing the file and writing anew
 *
 * Return Value
 *   int: >=0 Greater-than or equal-to zero value representing the bvfs file
 *           descriptor on success.
 *        -1 if some kind of failure occurred. Also, print a meaningful error to
 *           stderr prior to returning.
 */
int bvfs_open(const char *fileName, int mode) {
    if (inited == 1) {
        // First check to see if the file exists yet or not
        int fileDescriptor = findFileDesc(fileName);
        //printf("fileDescriptor in open: %d\n", fileDescriptor);

        // Index to access the Inode array
        int index = fileDescriptor - 1;

        // Check if the file exists 
        // That means the number given above is not equal to -1
        if (fileDescriptor != -1) { 
            // Check if the file is aready open
            if (INList[index].opened > 0) {
                fprintf(stderr, "The file <%s> is already open.\n", fileName);
                return -1;
            }

            // Now check the mode

            // Just open the file to read 
            if (mode == BVFS_RDONLY) {
                INList[index].opened = 1;
            }

            // Open the file so it is prepared to write to the end of it
            else if (mode == BVFS_WAPPEND) {
                INList[index].opened = 2;
            }

            // Open the file and delete everything in it
            else if (mode == BVFS_WTRUNC) {
                INList[index].opened = 3;
                
                // Delete everything from the file
                for(int i = 0; i < INList[index].numBlocks; i++) {
                    int deleteBlock = INList[index].dataBlockAddresses[i];
                    giveBackBlock(deleteBlock);
                }

                // Update the super block in the partition
                SB.remainingBlocks = SB.remainingBlocks + (INList[index].numBlocks - 1); 
                lseek(pFD, 0, SEEK_SET);
                write(pFD, (void*)&SB, sizeof(SB));

                // Create new data block address list to be put in the Inode
                uint16_t newAddresses[128];
                
                // Grab a new block that is empty and put it in the new address list
                uint16_t newBlock = getFreeBlock();
                newAddresses[0] = newBlock;

                // Put the new address list in the Inode 
                memcpy(INList[index].dataBlockAddresses, newAddresses, sizeof(newAddresses));

                // Set the numBlock back to one
                INList[index].numBlocks = 1;

                // Set the numBytes to zero
                INList[index].numBytes = 0;

                // Set the modification time 
                INList[index].lastModTime = time(NULL);
            }
            
            // Error because that is not a vaild open mode
            else {
                fprintf(stderr, "That is not a valid mode to open a file with: <%d>\n", mode);
                return -1;
            }
            
            // Write the INList back to file
            lseek(pFD, blockSize, SEEK_SET);
            write(pFD, (void*)&INList, sizeof(INList));

            // Return the proper file descriptor for the file opened
            return fileDescriptor; 
        }

        //######## Creating the file in open below ########//

        // Check if the mode is a write and not read and if there is a free file
        else {
            // Check if there is a free file first
            int isFree = findFreeFile();
            if (isFree == -1) {
                fprintf(stderr, "There are no free files left.\n");
                return -1;
            }

            // If it's readonly mode print to std error and return -1
            else if (mode == BVFS_RDONLY) {
                fprintf(stderr, "You cannot open a new file in read only mode.\n");
                return -1;
            }

            // If it's write, create the file and write the stuff
            else if (mode == BVFS_WAPPEND || mode == BVFS_WTRUNC) {
                // Set the isFree to the fileDescriptor and the correct value (+1)
                fileDescriptor = (isFree + 1);

                // Get the block where the data for this file will live
                uint16_t firstBlock = getFreeBlock();
                uint16_t addresses[128];
                addresses[0] = firstBlock;

                // Create the Inode
                Inode newNode = {.fd = fileDescriptor,
                                 .numBlocks = 1,
                                 .opened = 2,
                                 .lastDB = 0,
                                 .nextFreeByte = 0,
                                 .numBytes = 0,
                                 .lastModTime = time(NULL),
                };

                // Copy the arrays over
                strcpy(newNode.name, fileName);
                memcpy(newNode.dataBlockAddresses, addresses, sizeof(addresses));
                
                // Add the node to the INList
                INList[fileDescriptor - 1] = newNode;

                // Write the new updated Inode list to the partiton
                lseek(pFD, blockSize, SEEK_SET);
                write(pFD, (void*)&INList, sizeof(INList));

                // Update the super block
                SB.remainingFiles = SB.remainingFiles - 1;
                SB.remainingBlocks = SB.remainingBlocks - 1;

                // Update the super block in the partition
                lseek(pFD, 0, SEEK_SET);
                write(pFD, (void*)&SB, sizeof(SB));

                // Return successfully
                return fileDescriptor;
            }

            // Error because that is not a vaild open mode
            else {
                fprintf(stderr, "That is not a valid mode to open a file with: <%d>.\n", mode);
                return -1;
            }
        }
    }

    // Error that init was never called
    else {
        fprintf(stderr, "bvfs_init was never called.\n");
        return -1;
    }
}






/*
 * int bvfs_close(int bvfs_FD);
 *
 * This function is intended to close a file that was previously opened via a
 * call to bvfs_open. This will allow you to perform any finalizing writes needed
 * to the bvfs file system.
 *
 * Input Parameters
 *   fileName: A int representing the file descriptor of the file you wish to fetch
 *             (or create) in the bvfs file system.
 *
 * Return Value
 *   int:  0 if open succeeded.
 *        -1 if some kind of failure occurred (eg. the file was not previously
 *           opened via bvfs_open). Also, print a meaningful error to stderr
 *           prior to returning.
 */
int bvfs_close(int bvfs_FD) {
    int index = bvfs_FD - 1;

    if (inited == 1) {
        // First check to see if the file is actually open
        if (INList[index].opened == 0) {
            fprintf(stderr, "The file, <%s>, was not open.\n", INList[index].name);
            return -1;
        }
        else {
            INList[index].opened = 0;
            lseek(pFD, blockSize, SEEK_SET);
            write(pFD, (void*)&INList, sizeof(INList));
            return 0;
        }
    }

    // Error that init was never called
    else {
        fprintf(stderr, "bvfs_init was never called.\n");
        return -1;
    }

}







/*
 * int bvfs_write(int bvfs_FD, const void *buf, size_t count);
 *
 * This function will write count bytes from buf into a location corresponding
 * to the cursor of the file represented by bvfs_FD.
 *
 * Input Parameters
 *   bvfs_FD: The identifier for the file to write to.
 *   buf: The buffer containing the data we wish to write to the file.
 *   count: The number of bytes we intend to write from the buffer to the file.
 *
 * Return Value
 *   int: >=0 Value representing the number of bytes written to the file.
 *        -1 if some kind of failure occurred (eg. the file is not currently
 *           opened via bvfs_open). Also, print a meaningful error to stderr
 *           prior to returning.
 */
int bvfs_write(int bvfs_FD, const void *buf, size_t count) {
    int index = bvfs_FD - 1;
    int tempCount = count;

    // TODO Think about Checking if we can put all the stuff in the partition
    // Is the partition space left too small?????:w

    if (inited == 1) {
        // Check if the file is opened properly
        if (INList[index].opened == 2 || INList[index].opened == 3) {
            // Check if we can fit the stuff in the file
            if ((maxFileBytes - INList[index].numBytes) >= count) {
                // Check here to see if the current block is full
                // If it is, grab another here
                if (INList[index].nextFreeByte >= 512) {
                    // Grab a new block and move everything around
                    uint16_t newBlock = getFreeBlock();
                    
                    // Update the variables in the Inode
                    INList[index].nextFreeByte = 0;
                    INList[index].lastDB = INList[index].lastDB + 1;
                    INList[index].dataBlockAddresses[INList[index].lastDB] = newBlock;
                }

                // Loop through writing block at a time until all bytes are written
                while (tempCount != 0) {
                    // See how many bytes can fit in the current not full block
                    int freeBytes = blockSize - INList[index].nextFreeByte;
                    //printf("freeBytes: %d\n", freeBytes);

                    // If we can fit everything in the current block
                    if (tempCount <= freeBytes) {

                        // Seek to the spot to put data and write everything
                        int seekSpot = INList[index].dataBlockAddresses[INList[index].lastDB];
                        //printf("seekSpot: %d\n", seekSpot*blockSize);
                        //printf("seekSpot + offset: %d\n", (seekSpot*blockSize) + INList[index].nextFreeByte);

                        //printf("nextFreeByte: %d\n", INList[index].nextFreeByte);
                        lseek(pFD, (seekSpot*blockSize) + INList[index].nextFreeByte, SEEK_SET); 
                        write(pFD, buf, tempCount);

                        // Update where the nextFreeByte is in the Inode
                        INList[index].nextFreeByte = INList[index].nextFreeByte + tempCount;
                        
                        // Set tempCount to break the loop
                        tempCount = 0;
                    }

                    // We cannot fit everything in the current block
                    else {
                        // Write as much as the block can hold
                        int seekSpot = INList[index].dataBlockAddresses[INList[index].lastDB];
                        lseek(pFD, (seekSpot*blockSize) + INList[index].nextFreeByte, SEEK_SET); 
                        write(pFD, buf, freeBytes);

                        // Move buf up by how much was written
                        buf = buf + freeBytes;
                        
                        // Grab a new block and move everything around
                        uint16_t newBlock = getFreeBlock();
                        
                        // Update the variables in the Inode
                        INList[index].nextFreeByte = 0;
                        INList[index].lastDB = INList[index].lastDB + 1;
                        INList[index].dataBlockAddresses[INList[index].lastDB] = newBlock;
                        
                        // Set tempcount down the number of bytes being written
                        tempCount = tempCount - freeBytes;
                    }

                }
                // Increment the number of bytes and mod time in the file
                INList[index].numBytes = INList[index].numBytes + count;
                INList[index].lastModTime = time(NULL);

                // Write the Inode list back to partition
                lseek(pFD, blockSize, SEEK_SET);
                write(pFD, (void*)&INList, sizeof(INList));

                // Return the number of bytes written 
                return (int)count;
            }

            // Error that stuff to write is not going to fit.
            else {
                fprintf(stderr, "This file does not have room for that amount of bytes\n.");
                return -1;
            }
        }

        // Error that the file is not open or is in read mode
        else {
            fprintf(stderr, "The file, <%s>, is not open or is in read only mode\n", INList[index].name);
            return -1;
        }
    }

    // Error that init was never called
    else {
        fprintf(stderr, "bvfs_init was never called.\n");
        return -1;
    }
}






/*
 * int bvfs_read(int bvfs_FD, void *buf, size_t count);
 *
 * This function will read count bytes from the location corresponding to the
 * cursor of the file (represented by bvfs_FD) to buf.
 *
 * Input Parameters
 *   bvfs_FD: The identifier for the file to read from.
 *   buf: The buffer that we will write the data to.
 *   count: The number of bytes we intend to write to the buffer from the file.
 *
 * Return Value
 *   int: >=0 Value representing the number of bytes written to buf.
 *        -1 if some kind of failure occurred (eg. the file is not currently
 *           opened via bvfs_open). Also, print a meaningful error to stderr
 *           prior to returning.
 */
int bvfs_read(int bvfs_FD, void *buf, size_t count) {
    int index = bvfs_FD - 1;
    int tempCount = count;

    if (inited == 1) {
        // Check if the file is opened properly
        if (INList[index].opened == 1) {
            //TODO see if there is enough stuff to read from the file
            //That is, does the file have enough data in it

            // Int to keep track of which block we are reading from
            int blk = 0;

            //Find where the first block of info is in the file
            int firstBlock = INList[index].dataBlockAddresses[blk];

            // Seek to the block found above
            lseek(pFD, firstBlock*blockSize, SEEK_SET);
            
            while (tempCount != 0) {
                // See if we have to read more than 1 block of bytes
                // If we do, read entire block and seek to the next
                if (tempCount > 512) {
                    // Increment the blk to grab
                    blk = blk + 1;

                    // Read blockSize bytes to the buffer 
                    read(pFD, buf, blockSize);

                    // Decrement the tempCounter so we know how many bytes are left to read
                    tempCount = tempCount - blockSize;

                    // Increment the place in the buffer to put the data
                    buf = buf + 512;

                    // Seek to the next block
                    int nextBlock = INList[index].dataBlockAddresses[blk];
                    lseek(pFD, nextBlock*blockSize, SEEK_SET);
                }

                // Read what is left
                else {
                    // Read in what is left to the buffer
                    read(pFD, buf, tempCount);
                    // Set tempCount to 0 to break the loop
                    tempCount = 0;
                }
            }

            //Return the number of bytes read
            return count;
        }

        // Error that the file is not open or is in read mode
        else {
            fprintf(stderr, "The file, <%s>, is not open to read-only mode\n", INList[index].name);
            return -1;
        }
    }

    // Error that init was never called
    else {
        fprintf(stderr, "bvfs_init was never called.\n");
        return -1;
    }
}







/*
 * int bvfs_unlink(const char* fileName);
 *
 * This function is intended to delete a file that has been allocated within
 * the bvfs file system.
 *
 * Input Parameters
 *   fileName: A c-string representing the name of the file you wish to delete
 *             from the bvfs file system.
 *
 * Return Value
 *   int:  0 if the delete succeeded.
 *        -1 if some kind of failure occurred (eg. the file does not exist).
 *           Also, print a meaningful error to stderr prior to returning.
 */
int bvfs_unlink(const char* fileName) {
    //Increment SB.remaining files and blocks
    
    // First check if the partiton was inited
    if (inited == 1) {
        // Find the corresponding file descriptor
        int fileDescriptor = findFileDesc(fileName);    

        // Check to see if that is a valid file descriptor
        if (fileDescriptor != - 1) {
            // Set the index to reference the Inode with
            int index = fileDescriptor - 1;

            // Grab how many block will need to be given back
            int blocksBack = INList[index].numBlocks;

            // Loop blocksBack many times giving back the blocks
            for (int i = 0; i < blocksBack; i++) {
                giveBackBlock(INList[index].dataBlockAddresses[i]);
            }
        
            // Create an empty Inode
            Inode IN = {0,0,0,0,0,0,0,0,0,0};

            // Write empty Inode to the list and to the partiton
            INList[index] = IN;
            lseek(pFD, blockSize, SEEK_SET);
            write(pFD, (void*)&INList, sizeof(INList));
            
            // Update the number of files in the superblock
            SB.remainingFiles = SB.remainingFiles + 1;
            lseek(pFD, 0, SEEK_SET);
            write(pFD, (void*)&SB, sizeof(SB));

            // Return successfully
            return 0;
        }

        // Error that the file name was not found
        else {
            fprintf(stderr, "Filename <%s> was not found.", fileName);
            return -1;
        }
    }
    
    // Error that init was never called
    else {
        fprintf(stderr, "bvfs_init was never called.\n");
        return -1;
    }
}







/*
 * void bvfs_ls();
 *
 * This function will list the contests of the single-directory file system.
 * First, you must print out a header that declares how many files live within
 * the file system. See the example below in which we print "2 Files" up top.
 * Then display the following information for each file listed:
 *   1) the file size in bytes
 *   2) the number of blocks occupied within bvfs
 *   3) the time and date of last modification (derived from unix timestamp)
 *   4) the name of the file.
 * An example of such output appears below:
 *    | 2 Files
 *    | bytes:  276, blocks: 1, Tue Nov 14 09:01:32 2017, bvfs.h
 *    | bytes: 1998, blocks: 4, Tue Nov 14 10:32:02 2017, notes.txt
 *
 * Hint: #include <time.h>
 * Hint: time_t now = time(NULL); // gets the current unix timestamp (32 bits)
 * Hint: printf("%s\n", ctime(&now));
 *
 * Input Parameters
 *   None
 *
 * Return Value
 *   void
 */
void bvfs_ls() {
    // Calculate and print the number of files in the file system
    int numFiles = maxFiles - SB.remainingFiles;
    printf(" | %d File(s)\n", numFiles);
   
    // Create and array containing all the file indexes

    // Create an array to reference
    int files[256];

    // Index to add stuff to the files array at
    int fIndex = 0;

    // Search through all the Inodes to see if any have files in them
    for (int i = 0; i < maxFiles; i++) {
        if (INList[i].lastModTime != 0) {
            files[fIndex] = i;
            fIndex = fIndex + 1;
        }
    }

    // Loop numFiles times printing out info of each file
    for (int i = 0; i < numFiles; i++) {
        // Grab variables to print out
        int index = files[i];
        int bytes = INList[index].numBytes;
        int blocks = INList[index].numBlocks;
        time_t lastTime = INList[index].lastModTime;
        char* fName = INList[index].name;

        // Convert the time and take away the newline
        char* fileTime = ctime(&lastTime);
        fileTime[strlen(fileTime) - 1] = '\0';

        // Print file info to the screen
        printf(" | bytes: %d, blocks: %d, %s, %s\n", bytes, blocks, fileTime, fName);
    }
}
