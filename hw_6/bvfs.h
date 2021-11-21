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


// Struct to represent the Inodes in this file system
struct Inode { //TODO Add more stuff to this struct
    char fileName[32];
    uint8_t numBlocks;
    uint8_t numBytes;
    uint16_t dataBlockAddresses[128]; // Diskmap represented as index of the block
    // Meaning I need to multiply by blockSize to get actual location
    time_t lastModTime;
    char padding[212]; // Extra padding to fill the block size

} typedef Inode;


// Struct to represent the one super block in this file system
struct superBlock { //TODO possibly add more stuff here
    uint16_t remainingFiles;
    uint16_t firstFreeList; // Represented as an index of blocks
    char padding[508]; // Extra padding to fill the block size

} typedef superBlock;


//Struct to represent the empty space
struct freeSpace {
    uint16_t freeBlocks[255];
    uint16_t nextFreeSpaceBlock;

} typedef freeSpace;


//** Global Variables **//
uint16_t blockSize = 512;       // Reference to size of block in our file system
uint16_t maxFreeBlocks = 63;    // Most Free node we can have
uint16_t maxBlocks = 16384;     // Max number of block available in file system
superBlock SB;                  // Super block to manipulate later
freeSpace FS;                   // First free space block 


// Prototypes
int bvfs_init(const char *fs_fileName);
int bvfs_detach();
int bvfs_open(const char *fileName, int mode);
int bvfs_close(int bvfs_FD);
int bvfs_write(int bvfs_FD, const void *buf, size_t count);
int bvfs_read(int bvfs_FD, void *buf, size_t count);
int bvfs_unlink(const char* fileName);
void bvfs_ls();




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
    int pFD = open(fs_fileName, O_CREAT | O_RDWR | O_EXCL, 0644); // Last parameter is permissions
    if (pFD < 0) {
        if (errno == EEXIST) { // File already exists. 
            // Open it and read data for in-memory data structures 
            pFD = open(fs_fileName, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);

            // TODO change this stuff later and initalize in-memory data structures
            read(pFD, (void*)&SB, sizeof(SB));
            printf("Read from file: %d + %d + [%s]\n", SB.remainingFiles, SB.firstFreeList, SB.padding);

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
        superBlock SB = {256, 257, 0};
        write(pFD, (void*)&SB, sizeof(SB));

        // Create an array of Inodes
        Inode IN = {0,0,0,0,0,0};
        Inode INlist [256];
        
        // Write empty Inode blocks to array
        for (int i = 0; i < 256; i++) {
            INlist[i] = IN;
        }

        // Write Inode array to partition
        write(pFD, (void*)INlist, sizeof(INlist));        
        
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
                else {
                    nextFreeNode = i + 1;
                    freeList[freeListLoc] = i;
                    freeListLoc++;
                }
            }
            freeSpot += 258; // Takes me to the first place data can be stored after the new free node
            freeListLoc = 0; // Reset the location inside the freeList

            // Create the freeSpace struct
            printf("%d\n", freeList[0]);
            freeSpace FS = {*freeList, nextFreeNode};
            
            // Write the free block to file at the seeked location from above
            write(pFD, (void*)&FS, sizeof(FS));
        }

        printf("Created Partition: %s\n", fs_fileName);
    }

    return 0;
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
 * exist, you will create it. The function should return a bvfs file descriptor
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
}






/*
 * int bvfs_close(int bvfs_FD);
 *
 * This function is intended to close a file that was previously opened via a
 * call to bvfs_open. This will allow you to perform any finalizing writes needed
 * to the bvfs file system.
 *
 * Input Parameters
 *   fileName: A c-string representing the name of the file you wish to fetch
 *             (or create) in the bvfs file system.
 *
 * Return Value
 *   int:  0 if open succeeded.
 *        -1 if some kind of failure occurred (eg. the file was not previously
 *           opened via bvfs_open). Also, print a meaningful error to stderr
 *           prior to returning.
 */
int bvfs_close(int bvfs_FD) {
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
}
