#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bvfs.h"
///**********************************
//
//  MAKE SURE ALL THE WRITING IS NOT SET TO TRUNCATE AND ALWAYS CHECK OTHER 
//  VARIABLES LIKE THAT IF THEY COULD AFFECT FUNCTIONALITY
//
//******************


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <partition name to read/write>\n", argv[0]);
        return 1;
    }

    // Get partition name from command line argument
    char *partitionName = argv[1];

/////Initing
    int in = bvfs_init(partitionName);
    printf("init: %d\n", in);

/////File1
    int f1 = bvfs_open("file1", BVFS_WAPPEND);
    printf("open file1: %d\n", f1);

    int num = 123456789;
    int w1 = bvfs_write(f1, &num, sizeof(num));
    printf("write file1: %d\n", w1);

    int cl1 = bvfs_close(f1);
    printf("close file1: %d\n", cl1);

    int f1op = bvfs_open("file1", BVFS_RDONLY);
    printf("open file1 readonly: %d\n", f1op);

    int readNum;
    int read1 = bvfs_read(f1op, &readNum, sizeof(readNum));
    printf("read1: %d\n", read1);
    printf("value of read1: %d\n", readNum);

    cl1 = bvfs_close(f1op);
    printf("close fie1 again: %d\n", cl1);
    
/////File 2

    int f2 = bvfs_open("file2", BVFS_WTRUNC);
    printf("open file2: %d\n", f2);
    
    int cl2 = bvfs_close(f2);
    printf("close file2: %d\n", cl2);


/////Detaching
    int det = bvfs_detach();
    printf("detatch: %d\n", det);


    return 0;
}

